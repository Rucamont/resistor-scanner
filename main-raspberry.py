import cv2
import numpy as np
from keras.models import load_model
from PIL import Image, ImageOps
import bluetooth
from time import sleep
import socket
import RPi.GPIO as GPIO
import time

# Obtener el nombre del host de la Raspberry Pi
raspberry_pi_name = socket.gethostname()
print("Nombre de la Raspberry Pi:", raspberry_pi_name)

# Nombre del dispositivo ESP32
esp32_name = 'G4'  # Reemplaza con el nombre real definido en el codigo de la esp32

# Encuentra la dirección MAC del ESP32
devices = bluetooth.discover_devices()
esp32_mac_address = None

for addr in devices:
    print(addr)
    print('buscando dispositivo...')
    if bluetooth.lookup_name(addr) == esp32_name:
        esp32_mac_address = addr
        print('dispositivo encontrado...')
        print(bluetooth.lookup_name(addr))
        break
    sleep(1)

if esp32_mac_address is None:
    print('No se encontró el dispositivo ESP32.')
    exit()

# Crea un socket RFCOMM para la comunicación Bluetooth
socket = bluetooth.BluetoothSocket(bluetooth.RFCOMM)

# Establece la conexión con el ESP32
socket.connect((esp32_mac_address, 1))  # El segundo argumento es el canal (puerto) Bluetooth


# Configuración de pines GPIO
GPIO.setmode(GPIO.BCM)
D0_PIN = 17  # Ajusta el pin según la conexión
GPIO.setup(D0_PIN, GPIO.IN)


# Disable scientific notation for clarity
np.set_printoptions(suppress=True)

# Load the model
model = load_model("keras_model.h5", compile=False)

# Load the labels
class_names = [line.strip() for line in open("labels.txt", "r")]
rtsp_url = 'http://192.168.100.237:4747/video'
# Open the camera
camera = cv2.VideoCapture(rtsp_url)  # Use the appropriate camera index

# Variable to control image capture and processing
processing_enabled = False
send_blue =  True
try:
    while True:
        ret, frame = camera.read()

        cv2.imshow("Camera", frame)

        # Listen for key press
        keyboard_input = cv2.waitKey(1)
        if GPIO.input(D0_PIN) != GPIO.HIGH:
            print("Objeto detectado")
            # Capture the current frame
            image = Image.fromarray(frame)
            processing_enabled = True
        if keyboard_input == 27:
            break
        elif keyboard_input == 32:  # ASCII code for the Space key
            break

        if processing_enabled:
            # Resize and preprocess the image
            size = (224, 224)
            image = ImageOps.fit(image, size, Image.Resampling.LANCZOS)
            image_array = np.asarray(image)
            normalized_image_array = (image_array.astype(np.float32) / 127.5) - 1
            data = np.ndarray(shape=(1, 224, 224, 3), dtype=np.float32)
            data[0] = normalized_image_array

            # Make a prediction
            prediction = model.predict(data)
            index = np.argmax(prediction)
            class_name = class_names[index]
            confidence_score = prediction[0][index]
            if confidence_score >= 0.92:
                print("Class:", class_name, "Confidence Score:", confidence_score)
                cv2.putText(frame, class_name, (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 0), 2)
                if send_blue == True:
                   print("Se envio el mensaje: ", class_name)
                   socket.send(class_name)
                   send_blue= False


            else:
                send_blue = True
	    # Reset processing_enabled for the next capture
        processing_enabled = False


except KeyboardInterrupt:
    GPIO.cleanup()

camera.release()
cv2.destroyAllWindows()
