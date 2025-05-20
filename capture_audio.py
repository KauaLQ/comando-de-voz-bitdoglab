# python3 capture_audio.py COM6

import serial
import wave
import struct
import sys
import numpy as np
from collections import deque
import speech_recognition as sr

PORT = sys.argv[1] if len(sys.argv) > 1 else '/dev/ttyACM0'
SAMPLE_RATE = 8000
SAMPLE_WIDTH = 2  # bytes
DURATION_SEC = 10
TRIGGER_THRESHOLD = 20  # sensibilidade (pode ajustar)
TRIGGER_WINDOW = 100  # tamanho da janela para análise

ser = serial.Serial(PORT, 115200, timeout=1)

r = sr.Recognizer()

def transcrever_fala(caminho_arquivo):
    # Use sr.AudioFile para abrir o arquivo de áudio
    with sr.AudioFile(caminho_arquivo) as arq_audio:
        print("Ajustando o ruído ambiente...")
        # A função adjust_for_ambient_noise precisa de um objeto AudioSource.
        # arq_audio (retornado por sr.AudioFile) é um AudioSource.
        r.adjust_for_ambient_noise(arq_audio)

        print("Escutando...")
        # A função listen também precisa de um objeto AudioSource.
        audio = r.listen(arq_audio)

        print("Processando...")

        try:
            texto = r.recognize_google(audio, language='pt-BR')
            print("Você disse:", texto)
            enviar_serial(texto)
        except sr.UnknownValueError:
            print("Não entendi o que você disse.")
            enviar_serial("Incompreensivel")
        except sr.RequestError as e:
            print(f"Erro ao requisitar ao serviço de reconhecimento de fala: {e}")
            enviar_serial("Erro")

def enviar_serial(msg):
    ser.write((msg + '\n').encode('utf-8'))

print("[INFO] Esperando fala...")

# Janela deslizante para análise
window = deque(maxlen=TRIGGER_WINDOW)
recording = False
buffer = []

while True:
    raw = ser.read()
    if not raw:
        continue

    value = raw[0]
    diff = abs(value - 128)
    window.append(diff)

    # Se já estiver gravando
    if recording:
        signed = int((value - 128) * 256)
        buffer.append(signed)
        if len(buffer) >= SAMPLE_RATE * DURATION_SEC:
            # Finaliza e salva
            print("[INFO] Fala detectada. Gravando...")
            enviar_serial("Gravando...")

            with wave.open("voz.wav", "wb") as wf:
                wf.setnchannels(1)
                wf.setsampwidth(SAMPLE_WIDTH)
                wf.setframerate(SAMPLE_RATE)
                for s in buffer:
                    wf.writeframes(struct.pack('<h', s))

            print("[INFO] Áudio salvo como voz.wav")
            enviar_serial("Transcrevendo...")
            buffer.clear()
            recording = False

            caminho_voz = r"C:\Users\limak\OneDrive\Documentos\microphone_adc\voz.wav"
            transcrever_fala(caminho_voz)

            print("[INFO] Esperando nova fala...")
    else:
        # Se não estiver gravando, verifica se tem fala
        if len(window) == TRIGGER_WINDOW and np.mean(window) > TRIGGER_THRESHOLD:
            recording = True
            buffer.clear()
            print("[INFO] Atividade vocal detectada. Iniciando gravação...")
            enviar_serial("Gravando...")