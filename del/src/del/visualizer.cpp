#include "visualizer.hpp"

#include "graphviz/gvc.h"


namespace del
{
	visualizer & visualizer::get_instance()
	{
		static visualizer v;
		return v;
	}

	visualizer::visualizer() :
		gvc(gvContext())
	{

	}

	void visualizer::rendered_data_deleter::operator()(char * data)
	{
		gvFreeRenderData(data);
	}

	std::unique_ptr<char, visualizer::rendered_data_deleter> visualizer::visualize(state_id s)
	{
		char const * engine = "circo";

		char graph_name[] = "G";
		struct graph_deleter {
			void operator()(Agraph_t * g) { agclose(g); }
		};
		std::unique_ptr<Agraph_t, graph_deleter> graph(agopen(graph_name, Agdirected, NULL));

		if (gvLayout(this->gvc.get(), graph.get(), engine) != 0)
		{
			throw std::runtime_error("gvLayout returned error");
		}

		char * data = nullptr;
		unsigned int length = 0;
		if (gvRenderData(this->gvc.get(), graph.get(), engine, &data, &length) != 0)
		{
			if (data != nullptr) gvFreeRenderData(data); // Not sure what happens with the allocation if the function fails... Do we have to free?
			throw std::runtime_error("gvRenderData returned error");
		}
		
		return std::unique_ptr<char, visualizer::rendered_data_deleter>(data);
	}

	std::unique_ptr<char, visualizer::rendered_data_deleter> visualizer::visualize(action_id a)
	{
		return {};
	}

	void visualizer::context_deleter::operator()(GVC_t * gvc)
	{
		gvFreeContext(gvc);
	}
}
