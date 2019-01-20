FROM debian:latest

RUN apt-get update
RUN apt-get install libopenal1 -y
RUN apt-get install libsdl-sound1.2 -y
RUN apt-get install libncurses5 -y


ADD practical .
ADD servo2.wav .

CMD ./practical