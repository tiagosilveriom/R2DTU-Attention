import speech_recognition as sr


r = None

def initialize():
    global r
    r = sr.Recognizer()

def process():
    with sr.AudioFile("recording.wav") as source:
        audio = r.record(source)
        try:
            result = r.recognize_google(audio)
            return result.lower().replace("tube", "cube").replace("pills", "is").replace("from", "for").replace("q","cube").replace("kew", "cube").replace("key", "cube").replace("queue", "cube").replace("rib","red").replace("rubik's","red").replace("root", "red").replace("hanson", "Hansen").replace("hansen", "Hansen").replace("henderson", "Hansen").replace("hamilton", "Hansen")
        except sr.UnknownValueError:
            print("Google Speech Recognition could not understand audio")
        except sr.RequestError as e:
            print("Could not request results from Google Speech Recognition service; {0}".format(e))
        return ""
