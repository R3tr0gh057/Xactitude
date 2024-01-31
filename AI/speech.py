import pyttsx3
from flask import Flask
import threading
import time

app = Flask(__name__)

engine = pyttsx3.init('sapi5')
voices = engine.getProperty('voices')
engine.setProperty('voice', voices[1].id)

def speak(audio):
    engine.say(audio)
    engine.runAndWait()

@app.route('/start', methods=['GET'])
def start():
    threading.Thread(target=speak, args=("the event has started",)).start()
    engine.endLoop()
    return 'the event has started\n'

if __name__ == "__main__":
    app.run(host='0.0.0.0', port=3000)
