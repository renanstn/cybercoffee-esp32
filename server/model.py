from pydantic import BaseModel


class NotificationModel(BaseModel):
    sender: str
    message: str


class Pomodoro(BaseModel):
    time_in_minutes: str
