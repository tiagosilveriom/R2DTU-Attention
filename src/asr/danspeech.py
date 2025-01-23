import danspeech
from danspeech.pretrained_models import DanSpeechPrimary
from danspeech.language_models import DSL3gram
import pyaudio
import numpy as np

audio = None
recognizer = None
frames = []
stream = None

def initialize():
    global recognizer, audio
    model = DanSpeechPrimary()
    lm = DSL3gram()
    recognizer = danspeech.Recognizer(model=model, lm=lm, with_gpu=True)
    audio = pyaudio.PyAudio()

def recording_callback(in_data, frame_count, time_info, status):
    global frames
    frames.append(in_data)
    return in_data, pyaudio.paContinue

def start_recording():
    global stream
    stream = audio.open(format=pyaudio.paInt16, channels=1, rate=16000, input=True, frames_per_buffer=1024, stream_callback=recording_callback)
    stream.start_stream()

keywords = ["røde", "grønne", "blå", "klods", "Thomas", "Lasse", "tror"]

def select_most_likely(results):
    outputs = []
    for r in results:
        r = r.replace("thomas", "Thomas").replace("tomas", "Thomas").replace("tomme", "Thomas").replace("laset", "Lasse").replace("lasse", "Lasse").replace("læsset", "Lasse").replace("klodser", "klods er").replace("glodser", "klods er").replace("er tror","tror").replace("af den", "at den")
        score = 0
        for k in keywords:
            if k in r:
                score += 1
        outputs.append((score, r))
    selected = sorted(outputs, key=lambda x: -x[0])[0][1]
    return selected
def finish_recording():
    global stream, frames
    stream.stop_stream()
    data = np.frombuffer(b''.join(frames), dtype=np.int16).astype(float)
    results = recognizer.recognize(data, show_all=True)
    frames = []
    return select_most_likely(results)

def shutdown():
    global recognizer
    recognizer = None
