from pydantic import BaseModel


class NotificationModel(BaseModel):
    sender: str
    message: str


class Timer(BaseModel):
    time_in_minutes: str
