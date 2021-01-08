from settings import BEEBOTTE_API_KEY, BEEBOTTE_SECRET_KEY
from model import NotificationModel
from fastapi import FastAPI
from beebotte import BBT, Resource


app = FastAPI()

@app.get('/')
async def root():
    return {"message": "hello!"}

@app.post('/notify/')
async def notify(notification: NotificationModel):
    message = f"Notification from {notification.sender}: "
    message += f"{notification.message}"
    try:
        beebotte_client = BBT(BEEBOTTE_API_KEY, BEEBOTTE_SECRET_KEY)
        resource = Resource(beebotte_client, "esp32", "notification")
        resource.publish(message)
        return {'message': 'Notification sent!'}
    except:
        return {'message': 'Error!'}
