#include <core/Node.h>

#include <queue>
#include <unordered_map>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <thread>
#include <mutex>
#include <condition_variable>

#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <signal.h>
#include <atomic>

#include <core/Module.h>
#include <core/ModuleLoader.h>
#include <core/Network.h>
#include <common/Message.h>
#include <common/Timer.h>

#include <msgs/CoreMsgs.h>

//TODO Thread safety subscribe, unsubscribe, advertise, deadvertise
//TODO Fix memory leak of received messages!
//Implement "Pause"

static volatile std::atomic<uint8_t> should_halt;
static volatile std::atomic<uint8_t> interrupt_count;

void sigint_handler(int sig) {
    signal(sig, SIG_IGN);
    should_halt = 1;
    if (interrupt_count++) exit(-1);
    signal(SIGINT, sigint_handler);
}


struct NodeModule {
    std::string name;
    Module* mod;
    uint32_t frequency;
    void* state;
};

struct Context {
    Node* node;
    std::vector<NodeModule*> modules;
    std::vector<std::thread*> worker_threads;

    //std::unordered_map<uint16_t, Publisher*> network_pubs;
    //std::vector<std::pair<Subscriber*, MessageType >> network_subs;

    std::unordered_map<std::string, uint32_t> kv_store;

    std::mutex kv_store_mutex;
    Publisher* kv_pub;
    Subscriber* kv_sub;

    Publisher* lifecycle_pub;
    Subscriber* lifecycle_sub;
    std::atomic<bool> running;
};

static Context ctx;

void load_module(const char* name, uint32_t frequency) {
    auto m = new NodeModule();
    m->name = name;
    m->mod = ModuleLoader::load(name);
    m->frequency = frequency;
    ctx.modules.push_back(m);
}



struct WorkerThreadParameters {
    Node* node;
    NodeModule* mod;
    sem_t* barrier;
    int* workers_not_ready;
    std::mutex* worker_mut;
    std::condition_variable* workers_ready;
};

void* run_module(WorkerThreadParameters* param) {
    auto nmod = param->mod;

    auto timer = Timer::create_fixed_frequency(nmod->frequency);

    nmod->state = nmod->mod->initialize(param->node);
    if (!nmod->state) {
        printf("Failed to init module %s\n", nmod->name.c_str());
        return nullptr;
    }

    //Sync with other workers
    param->worker_mut->lock();
    *param->workers_not_ready -= 1;
    if (*param->workers_not_ready == 0) {
        param->workers_ready->notify_one();
    }
    param->worker_mut->unlock();
    sem_wait(param->barrier);

    //Inner loop
    while (ctx.running) {
        nmod->mod->tick(param->node, nmod->state);
        Timer::sleep(&timer);
    }
    printf("%s shutting down\n", nmod->name.c_str());

    nmod->mod->shutdown(param->node, nmod->state);
    printf("%s done\n", nmod->name.c_str());
    ModuleLoader::unload(nmod->mod);
    return nullptr;
}


void Node::network_tick() {

    if (!Network::is_initialized()) return;

    for (auto p : network_subs_) {

        auto channel = p->owner_->channel_id_;

        auto opt = p->next();

        while (opt.is_some()) {
            std::shared_ptr<MessageHeader> msg = *opt;
            msg->channel = channel;

            auto packet = Network::pack_message(msg);
            Network::transmit(packet, 0);
            opt = p->next();
        }
    }
    Network::tick();
    auto& q = Network::access_received();

    while (!q.empty()) {
        auto header = q.front();
        auto search = publishers_by_channel_.find(header->channel);
        if (search != publishers_by_channel_.end()) {
            auto pub = search->second;
            for (auto& sub : pub->subscribers_) {
                if (sub->networked_) continue;
                sub->push(header);
            }
        }
        q.pop();
    }
}

void Node::internal_tick() {
    //Tick lifecycle
    {
        auto opt = ctx.lifecycle_sub->next();

        while (opt.is_some()) {
            auto msg = Messages::content<LifecycleMsg>(opt->get());
            switch (msg->type) {
                case LifecycleMsg::Type::START: ctx.running = true; break;
                case LifecycleMsg::Type::STOP: ctx.running = false; break;
            }
            opt = ctx.lifecycle_sub->next();
        }
    }
}

void Node::spin() {

    //Wait for start signal (All nodes have connected)
    while (!ctx.running && !should_halt) {
        network_tick();
        internal_tick();
        usleep(100000);
    }
    if (should_halt) return;
    //Setup worker threads
    printf("Starting modules\n");

    sem_t barrier;
    sem_init(&barrier, 0, 0);

    std::mutex worker_mut;
    std::condition_variable workers_ready;

    int workers_not_ready = (int) ctx.modules.size();

    auto params = (WorkerThreadParameters*) malloc(sizeof(WorkerThreadParameters) * ctx.modules.size());
    for (uint32_t i = 0; i < ctx.modules.size(); i++) {
        params[i].node = ctx.node;
        params[i].mod = ctx.modules[i];
        params[i].workers_not_ready = &workers_not_ready;
        params[i].workers_ready = &workers_ready;
        params[i].worker_mut = &worker_mut;
        params[i].barrier = &barrier;
        auto mem = malloc(sizeof(std::thread));
        auto t = new(mem) std::thread(run_module, &params[i]);
        ctx.worker_threads.push_back(t);
    }

    {
        std::unique_lock<std::mutex> lock(worker_mut);
        workers_ready.wait(lock, [&workers_not_ready] { return workers_not_ready == 0; });
    }

    for (uint32_t i = 0; i < ctx.modules.size(); i++) {
        sem_post(&barrier);
    }

    printf("All modules up and running!\n");


    //Node inner loop
    while (ctx.running && !should_halt) {
        usleep(10 * 1000);
        network_tick();
        internal_tick();
    }

    //Teardown workers
    printf("Shutting down\n");
    ctx.running = false;

    MessageBuilder<LifecycleMsg> msg;
    msg->sender = ctx.node->id;
    msg->type = LifecycleMsg::Type::STOP;

    ctx.lifecycle_pub->publish(msg);
    network_tick();
    for (auto t : ctx.worker_threads) {
        t->join();
        free(t);
    }
    free(params);
}


void Node::initialize(std::string const& config_name, bool is_leader, const char* leader_ip) {
    interrupt_count = 0;
    signal(SIGINT, sigint_handler);

    if (leader_ip != nullptr) {
        Network::initialize();
        if (is_leader) {
            Network::start_host(leader_ip, 1234);
        } else {
            Network::connect(leader_ip, 1234);
        }
        kv.init("192.168.1.101", 6379);
    } else {
        kv.init("127.0.0.1", 6379);
    }


    this->load_module = ::load_module;

    network_advertise<LifecycleMsg>("lifecycle");

    ctx.node = this;
    ctx.lifecycle_pub = advertise<LifecycleMsg>("lifecycle");
    ctx.lifecycle_sub = subscribe<LifecycleMsg>("lifecycle");

}

void Node::shutdown() {
    for (auto mod : ctx.modules) {
        delete mod;
    }
    kv.shutdown();
    Network::shutdown();
}

void Node::start() {
    MessageBuilder<LifecycleMsg> msg;
    msg->type = LifecycleMsg::Type::START;
    msg->sender = id;
    ctx.lifecycle_pub->publish(msg);
}
