# R2DTU

## Links

- [NaoQi SDK documentation](http://doc.aldebaran.com/2-5/index_dev_guide.html)
- [Things I wish I had known when I first started working with the Softbanks Pepper robot](https://github.com/MAvis-DTU/R2DTU)
- [Implementing Theory of Mind on a Robot Using Dynamic Epistemic Logic](https://doi.org/10.24963/ijcai.2020/224)
- [DEL-based Epistemic Planning for Human-Robot Collaboration: Theory and Implementation](http://www.imm.dtu.dk/~tobo/main_kr21.pdf)

## Information about our robots

Owner | [Version](http://doc.aldebaran.com/2-5/family/pepper_technical/pepper_versions.html) | IP | Password
--- | --- | --- | ---
DTU Compute | 1.8a | 192.168.0.102 | r2dtu
DTU Management | 1.8a | 192.168.0.104 | Sokrates1

Both robots will on boot connect to the R2DTU wireless network. The above IP addresses are automatically assigned by the router DHCP server.
The lab desktop is likewise assigned an IP address of 192.168.0.103

## Setting up the development environment

Automated setup and build scripts exists but due to fragile dependencies it __only__ works on Ubuntu 18.04 (out-of-the-box that is; it can of cause be made to work on other distributions/OSs)

Due to the closed source nature of the robot, multiple tarballs needs to
be downloaded manually, and placed into the "depend" folder.

Filename | md5 | link
--- | --- | ---
naoqi-sdk-2.5.5.5-linux64.tar.gz | 6168a4da3cabb1030ada3a99d954708e |https://community-static.aldebaran.com/resources/2.5.10/NAOqi%20SDK/naoqi-sdk-2.5.7.1-linux64.tar.gz
ctc-linux64-atom-2.5.2.74.zip | 82bff07795a2a4090b6b536a7ca5193e |https://community-static.aldebaran.com/resources/2.5.10/CTC/ctc-linux64-atom-2.5.10.7.zip

### Docker installation
This option uses docker to run the project in an isolated container, thus allowing for an easy setup and deployment.
The sole required host dependency is a newish Nvidia driver (tested with 430.40).
To install docker and setup the container run
```
$ ./scripts/docker_install
$ ./scripts/docker_build
```
The container can then be booted using
```
$ ./docker_start
```

### Local setup
A local setup is a bit more involved.
First install
+ cuda-10.1
+ cudnn-7

Then install all remaining dependencies using
```
$ git submodule update --init --recursive
$ sudo -H ./scripts/bootstrap
$ ./scripts/setup
```

Then when ever a rebuild is needed simply run
```
$ scripts/build
```
This repository is an extension of the work developed by **Thomas Bolander** and **Lasse Dissing**, which is available at [MAvis-DTU/R2DTU](https://github.com/MAvis-DTU/R2DTU). Please note that the linked repository may not be accessible due to privacy restrictions.

The **Dynamic Epistemic Logic (DEL)** implementation used in this project is linked to an external repository, where the implementation is available.

## Repository Structure

This repository contains **two branches**, each corresponding to one of the domains used for the experiments conducted to test this project.

by: **Tiago Silverio s222963**