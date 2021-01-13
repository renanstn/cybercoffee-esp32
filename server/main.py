import logging
from settings import BEEBOTTE_API_KEY, BEEBOTTE_SECRET_KEY
from model import NotificationModel, SetModeModel
from fastapi import FastAPI
from beebotte import BBT, Resource
from unidecode import unidecode


app = FastAPI()

@app.get('/')
async def root():
    return {"message": "hello!"}

@app.post('/notify/')
async def notify(notification: NotificationModel):
    message = f"Notification from {notification.sender}: "
    message += f"{notification.message}"
    logging.info(f"Mensagem recebida: {message}")
    # Remover acentos, pois a lib do display do arduino não os aceita
    message = unidecode(message)
    try:
        beebotte_client = BBT(BEEBOTTE_API_KEY, BEEBOTTE_SECRET_KEY)
        resource = Resource(beebotte_client, "esp32", "notification")
        resource.publish(message)
        return {'message': 'Notification sent!'}
    except:
        return {'message': 'Error!'}

@app.post('/set-mode/')
async def set_mode(set_mode: SetModeModel):
    modes = {
        "clock": 1,
        "timer": 2,
        "motor": 3,
    }
    mode = set_mode.mode
    if mode not in modes.keys():
        return {'message': 'Error! Mode not known'}
    numeric_mode = modes[mode]
    try:
        beebotte_client = BBT(BEEBOTTE_API_KEY, BEEBOTTE_SECRET_KEY)
        resource = Resource(beebotte_client, "esp32", "mode")
        resource.publish(numeric_mode)
        return {'message': 'Mode changed!'}
    except:
        return {'message': 'Error!'}
