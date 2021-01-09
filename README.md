# cybercoffee-esp32

<p float="left">
<img src="https://github.com/renanstd/cybercoffee-esp32/blob/main/screenshots/print01.jpg" width="360" height="360"/>
<img src="https://github.com/renanstd/cybercoffee-esp32/blob/main/screenshots/print02.jpg" width="360" height="360"/>
</p>

### Mas que p**** é essa?

Cybercoffe é um acessório revolucionário e absolutamente desnecessário que irá se acoplar a sua fiel caneca de café, e lhe fornecerá informações valiosas como hora, notificações recebidas no celular, timer pomodoro, alarmes sonoros, e até mesmo mexer seu café ela vai fazer!

>"É o futuro, na beira da sua caneca, fazendo peso a toa!"

Em desenvolvimento, pq eu não tenho nada melhor pra fazer na vida.

# Stack

Até o atual momento, as seguintes tecnologias foram envolvidas no desenvolvimento do Cybercoffee:

- Placa **ESP32**, com visor OLED (futuramente um piezo para sinais sonoros e um micro servo para mexer o café)
- Aplicativo [IFTTT](https://play.google.com/store/apps/details?id=com.ifttt.ifttt&hl=pt_BR&gl=US), instalado no Android, para enviar as notificações recebidas para a API via *webhook*
- API, feita em Python(:heart:) + [fastAPI](https://fastapi.tiangolo.com/), hospedada no [Heroku](https://www.heroku.com/)
- Serviço [Beebotte](https://beebotte.com/home) (conta gratuita), para fazer a ponte entre as requests da API e o protocolo MQTT utilizado pelo ESP32
- [Arduino IDE](https://www.arduino.cc/en/software) para carregar códigos na placa ESP32
