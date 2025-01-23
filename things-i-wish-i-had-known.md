# Things I wish I'd known when I first started working with the Softbanks Pepper Robot

During the years I have accumulated a lot of knowledge related to working with Softbanks Pepper Robots and their many particularities. This document is my attempt to write down some of this knowledge with the hope that this knowledge can be useful to you and help you avoid some of the many pitfall which I encountered during my work. 

## Workflows

### Application architecture

When developing an application for the Pepper robot, there is atleast two important computers involved: The 'onboard' computer which is placed in the robot's head and the 'host'/'field'/'development' computer which is the laptop or desktop from which you are developing the application. With just these two computers we can build a plenthora of different architectures, but the three that I have used are in chronological order:

- All code runs on the development PC, e.g., a Python script which remotely calls the NaoQI SDK. This is by far the simplest way to get started and is what I used for the very first demos. The primary disadvantage of this approach is however quite poor performance due to the SDK having terrible networking performance, especially with regards to tranfer of video.
- All code runs onboard the robot, e.g., a Linux C++ executable running on the robot. This alleviates most of the networking issues, but the robot's CPU is fairly slow and it also complicates development, i.e., there is no way to show the observed video on the development PC and debugging through a SSH session is not particularily fun.
- A client-server architecture where a small server running on the robot exposes the relevant functionality through an optimized network protocol. A client executable running on the development PC can then perform all of the computational heavy and frequently changing work. In my experience, this approach provides by far the best performance / ease of development tradeoff. The only downside of this approach is additional amount of work needed to implement this. Both the R2DTU False Belief code base and the MAvis-3 codebase uses this architecture.

### Installation

All projects I have work on with the robot have used the Linux NAOqi SDK which can be downloaded from [here](https://developer.softbankrobotics.com/pepper-naoqi-25-downloads-linux).
They also make this SDK for Windows and OS X, but I have no experience using these. 
The NAOqi SDK is being depreciated in favor of a new SDK called QiSDK which replaces the Linux-based C++/Python development environment with an Android-based Java/Kotlin based environment. I am quite sceptic of this switch to Android and Java, but I do not have concrete experience working with QiSDK and will therefore refrain from commenting on how it compares with the old NAOqi.
The NAOqi SDK consists of three relevant packages

- The C++ SDK which contains headers for the SDK and is therefore necesary for any C++ development.
- The Python SDK which allows Python scripts running on your development PC to remotely call SDK functions. This is great for experiments and quick scripts, but way to slow for any serious application.
- The 'Cross Toolchains' which contains: 1) A gcc compiler which compiles for x86. 2) pre-compiled libraries which your application must be linked to.

For developing C++ applications running onboard the robot itself, both the C++ SDK and the 'Cross Toolchains' are necessary.
A guide to installing the C++ SDK can be found [here](https://developer.softbankrobotics.com/pepper-naoqi-25/naoqi-developer-guide/sdks/c-sdk).
The guide uses their custom qiBuild system which is essentially just CMake and some extra commands. I used this build system initially and it is okay-ish, but I later replaced it with standard CMake in order to simplify installation.


### Cross-compilation

While the onboard CPU is a fairly standard x86-64 cpu, the Liunx distro installed is only 32-bit.
My guess as to why they have decided to use a 32-bit os is that it reduced the size of pointers from 8 to 4 bytes and since the robot only has 4 GB of ram, a 32-bit address space is sufficient.
The unfortunate consequences of this is however that it results is reduced CPU performance due to the fewer registers available in 32-bit mode and it greatly complicates the process of compiling code for the robot since an executable compiled on a standard 64-bit Linux distro will not run on the robot. Instead it must be [cross-compiled](https://en.wikipedia.org/wiki/Cross_compiler), that is, compiled with a target architecture distinct from that of the host machine. 


### Uploading code to the robot and running it

### Working with the tablet

While it would have been obvious to have the tablet mounted on the robot be a regular display connected to the onboard Linux computer, Softbanks instead opted to have the tablet be a separate Android-powered device which runs a browser session. The NaoQi tablet API then provides a few functions for sending commands to the browser. For this reason, showing custom content on the tablet requires that the content be exposed through some web service accessible to tablet. For simple content such as static images, the robot runs its own onboard web server which serves content from the `/opt/aldebaran/www/` folder on the robot. The tablet can then access it through the URL `http://198.18.0.1/`. E.g., suppose we have some file `image.png` which we want to show on the tablet. We can then copy this file to `/opt/aldebaran/www/image.png` and then have the tablet show it by running `tablet_service.showImage('http://198.18.0.1/file.png)`.  For more complex content, e.g., an interactive UI, you will need to implement the UI as a webapp which then communicates with your primary application through some HTTP API.

## Important NaoQi concepts

### Stiffness

'Stiffness' controls how much power the motors are allowed to use. This is not only relevant for performing dynamic movement but also for statically holding the motors against some external force, e.g., gravity. Consider for instance a situation where the robot has its arms in a horizontal position, pointing straight forward. The motors must then use a little power in order to counteract the gravitational force pushing down on the arms. The stiffness controls how much current the motor controllers are allowed to use for this. If motor performance was our only concern, we should just always use max stiffness, but that would result in unnecessary additional power consumption and heat generation. For this reason, the robot's built-in control systems automatically modulates the stiffness such as to ensure that it is no greater than needed and for vast majority of use cases you do not need to worry about this. However, in some specific use cases where high motor performance is desired, it might be necessary to override this stiffness using the [ALMotion](http://doc.aldebaran.com/2-4/naoqi/motion/control-stiffness.html) api.

### WakeUp and Rest

In general, you should store the robot in its 'sleeping position' when powered-off (unless it is in the box) since it is naturally stable and unlikely to fall over.
When starting your robot application, you can then use the `motion.wakeUp()` which does the following:
- Enables motor stiffness (which is zero on boot)
- Runs a quick self-test of motors and sensors

## Limitations

### Root / admin access to the robot
For unknown reasons, Softbank Robotics has decided against releasing the root password. [Source](https://github.com/RoboCupAtHome/RuleBook/issues/365)
It is therefore not possible to install system software, change hardware rules, update the clock, etc.
The only command I have found possible to run using sudo is `sudo halt` to powerdown the robot.
While this is incredible annoying, it is however still possible to install most software through cross-compilation and then uploading it through ssh.
Even something as complex as ROS can be installed through cross-compilation. [Link](https://github.com/esteve/ros2_pepper)


### Onboard cameras

After having worked with the robot for some time, you realize that during development of the robot, Softbanks clearly prioritized the robot's appearance and a low BOM cost over actual performance. This is especially evident with regards to the onboard cameras.
- The robot has two video cameras: A forward looking camera mounted on the forehead and a downward looking camera mounted in its mouth. The field-of-view of these two cameras _almost_ overlaps, but there is still a small gap of a couple of degrees. This significantly complicates the tracking of objects moving between the two camera views.
- The quality of these two camera is roughly comparable to that of an old webcam, i.e., it is fairly low resolution (640x480) and is very sensitive to the amount of light exposure.
- The robot is also equipped with a structured light depth sensor, which is so poorly designed that it is almost useless. In brief, this depth sensor is offset along all three axis relative to the forward video camera in such a way that aligning the two views results in a significantly warped image. It is therefore very difficult to corelate any particular pixel in the video stream with the corresponding depth value. In summary: If you want to obtain true RGB+D you should definetly use the RealSense cameras instead. 
- All three cameras are mounted on the head of the robot. While this might seem like a sensible choice at first glance, it has the very unfortunate consequence of introducing motion blur into the video stream whenever the robot moves its head. When combined with the fairly long exposure time used by the video cameras, this results in significantly degraded images. Thus, make sure to move the head slowly or step-wise when using it for object tracking.


### Video networking performance

The default NaoQi API for capturing video streams every image over the network using a poorly compressed XML serialization format. This means that the NaoQi API is only able to stream 640x480 video at ~5-10 fps. For this reason I wrote a custom user-space camera driver which instead compresses each individual image using JPEG prior to sending it over the network, easily allowing the system to stream 640x480 at 30 fps from both cameras concurrently. If you need real-time video, I highly recommend reusing this or some similar driver.
Note that it is of cause possible to optimize this even further by using a video compression algorithm, e.g. x264, instead of compressing each video frame individually, but so far this has not been needed.

## Bugs

### Collision avoidance

`ALMotion` is somewhat intelligent in that it modifies a motion command if it predicts that there is a significant risk of the motion resulting in the robot hitting either itself or some nearby object, e.g., if you command the robot to touch itself, the motion will stop a few centimeters away. These checks are referred to as [self-collision avoidance](http://doc.aldebaran.com/2-4/naoqi/motion/reflexes-collision-avoidance.html) and the [external-collision avoidance](http://doc.aldebaran.com/2-4/naoqi/motion/reflexes-collision-avoidance-api.html). In my experience, the self-collision avoidance is fairly good and in-fact necesary for some of the provided animations. I have only needed to disable this in cases where I explicitly needed the robot to touch itself, e.g. a clapping animation. In this case you need to be extra careful not to damage the robot, e.g. for the clapping animation I had the motion stop just short of the robot actually touching itself in order to avoid damaging the hands. In contrast the the external-collision avoidance is in my experience not only useless but in fact actively harmful. The robot uses an ensemble of [laser line detectors](http://doc.aldebaran.com/2-5/family/pepper_technical/laser_pep.html) and [sonars](http://doc.aldebaran.com/2-5/family/pepper_technical/sonar_pep.html) to detect external objects, but both of these sensor types are notoriously low-resolution meaning that: 1) The sensors is very likely to miss thin obstacles such as chair legs and 2) the robot cannot really determine whether a detected object is actually an obstacle for some particular motion. In practice, I have observed that the robot will often gladly drive into chairs and tables, but will completely freeze and refuse to make any movement at all if a human stands nearby. For this reason I recommend disabling the external-collision avoidance.



### NaN speeds

The NaoQi movement system implements safety checks which prevents the robot from moving to fast, i.e., if you command it to drive forward at 5000 m/s it will automatically clamp it to some sensible max safe value. There is however a bug where if you pass a 'non-a-number' float as the speed, it will drive the motors at max possible power which can cause the robot to lose its balance. I have only observed this bug when driving the wheel base (both for translational and rotational movement), but a similar bug might exists for moving the arm and head joints.

### Angle interpolation

http://doc.aldebaran.com/2-5/naoqi/motion/control-joint-api.html#ALMotionProxy::angleInterpolation__AL::ALValueCR.AL::ALValueCR.AL::ALValueCR.bCR
http://doc.aldebaran.com/2-5/naoqi/sensors/dcm_advanced.html

## Troubleshooting

### Robot runs the install behaviour on start-up
While I have disabled the 'Alive by default' option on both robots, the Socrates Pepper robot sometimes reenables this option for unknown reasons.
To fix this follow [these instructions](http://doc.aldebaran.com/2-5/family/pepper_user/guide/freeze_pep.html)
In brief: Press the chest button twice.

### The robot refuses to move its motors and complains about heat
The robots does not have sufficient cooling capacity to keep its motors running continously. Especially holding the motors in some position where they need to counteract significant gravitational force, e.g. holding the arms in a horizontal position, can quickly cause the motors to overheat. When such an overheating event occurs, the robot will turn down the 'stiffness' of the motors, causing them to loose power and go limp. In this case, there is really nothing to do but to wait for a few minutes and try to avoid repeating the forces on the motors which caused this overheating event.

### Robot complains about its laser sensors when exposed to strong sunlight.
During the `wakeUp` routine, the robot runs a quick self-test of all its motors and sensors. One of these tests includes checking whether its [line laser sensors](http://doc.aldebaran.com/2-4/family/pepper_technical/laser_pep.html) works. These lasers projects IR light down onto the floor which is then picked up by a simple IR camera. You can see these lines for yourself by using a phone camera (note that some phone cameras have IR filters which removes these lines). The self-test fails if the camera cannot see the generated line. This can occur if some bright light source, e.g. the sun, drowns out the laser light. Only way to avoid this is to either: 1) create a shadow blocking the light source or 2) skip the wake up routine.

