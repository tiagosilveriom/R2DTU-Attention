import qi
import time
import random
import sys

session = qi.Session()
session.connect("tcp://192.168.0.102:9559")
leds = session.service("ALLeds")
player = session.service("ALAudioPlayer")
behavior = session.service("ALBehaviorManager")
#text = session.service('ALTabletService')

animations = [
    "animations/Stand/Gestures/Thinking_1",
    "animations/Stand/Gestures/Thinking_3",
    "animations/Stand/Gestures/Thinking_4",
    "animations/Stand/Gestures/Thinking_6",
    "animations/Stand/Gestures/Thinking_8",
]


if len(sys.argv) == 2:
    if sys.argv[1] == "start":
        player.playFile("/opt/aldebaran/share/naoqi/wav/begin_reco.wav")
        leds.fadeRGB("FaceLeds", 0.0, 1.0, 0.0, 0.4)
        behavior.startBehavior(random.choice(animations))
        #text.hideDialog()
    elif sys.argv[1] == "stop":
        player.playFile("/opt/aldebaran/share/naoqi/wav/end_reco.wav")
        leds.fadeRGB("FaceLeds", 0.0, 0.0, 1.0, 0.4)

