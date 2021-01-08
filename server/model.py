from pydantic import BaseModel


class NotificationModel(BaseModel):
    sender: str
    message: str
