import logging
from settings import BEEBOTTE_API_KEY, BEEBOTTE_SECRET_KEY
from model import NotificationModel, Pomodoro
from fastapi import FastAPI
from beebotte import BBT, Resource
from unidecode import unidecode


app = FastAPI()

@app.get('/')
async def root():
    return {"message": "hello!"}

@app.post('/notify/')
async def notify(notification: NotificationModel):
    message = f"{notification.sender}: {notification.message}"
    logging.info(f"Mensagem recebida: {message}")
    # Remover acentos, pois a lib do display do arduino não os aceita
    message = unidecode(message)
    try:
        beebotte_client = BBT(BEEBOTTE_API_KEY, BEEBOTTE_SECRET_KEY)
        resource = Resource(beebotte_client, "esp32", "notification")
        resource.publish(message)
        return {'message': 'Notification sent'}
    except Exception as err:
        return {'message': str(err)}

@app.post('/start-pomodoro/')
async def set_mode(pomodoro: Pomodoro):
    time_in_minutes = pomodoro.time_in_minutes
    time_in_seconds = int(time_in_minutes) * 60
    try:
        beebotte_client = BBT(BEEBOTTE_API_KEY, BEEBOTTE_SECRET_KEY)
        resource = Resource(beebotte_client, "esp32", "pomodoro")
        resource.publish(time_in_seconds)
        return {'message': 'Pomodoro started'}
    except Exception as err:
        return {'message': str(err)}
