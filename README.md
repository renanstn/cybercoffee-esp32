# cybercoffee-esp32

![photo](https://github.com/renanstd/cybercoffee-esp32/blob/main/images/photo01.jpg)

### Mas que p**** é essa?

Cybercoffe é um acessório revolucionário e absolutamente desnecessário que irá se acoplar a sua fiel caneca de café, e lhe fornecerá informações valiosas como hora, notificações recebidas no celular, timer pomodoro com alarme sonoro, e até mesmo mexer seu café ela vai fazer!

>*"É o futuro, na beira da sua caneca, fazendo peso a toa!"*

Em desenvolvimento, pq eu não tenho nada melhor pra fazer na quarentena.

# Stack

Até o atual momento, as seguintes tecnologias foram envolvidas no desenvolvimento do Cybercoffee:

- Placa **ESP32**, com visor OLED, um piezo para sinais sonoros e um micro servo para mexer o café (achou que era zoeira? Achou errado, otário!)
- API REST, feita em Python(:heart:) + [fastAPI](https://fastapi.tiangolo.com/), hospedada no [Heroku](https://www.heroku.com/)
  - Esta API é responsável por receber comandos e notificações do celular, via *request HTTP*, tratá-las, e encaminhá-las para o Beebotte.
- Aplicativo [IFTTT](https://play.google.com/store/apps/details?id=com.ifttt.ifttt&hl=pt_BR&gl=US), instalado no Android
  - Este app automatiza o processo de enviar todas as notificações recebidas para a API via *webhook*
- Serviço [Beebotte](https://beebotte.com/home)
  - Este serviço faz a conexão do ESP32 na internet, via protocolo MQTT. Ele possui uma API *REST* que transforma as mensagens recebidas em publicações no sistema de *pub/sub* do MQTT
- [Arduino IDE](https://www.arduino.cc/en/software)
  - Para carregar códigos na placa ESP32

# Fotos

<p float="left">
<img src="https://github.com/renanstd/cybercoffee-esp32/blob/main/images/photo02.jpg" height="360"/>
<img src="https://github.com/renanstd/cybercoffee-esp32/blob/main/images/photo03.jpg" height="360"/>
</p>
