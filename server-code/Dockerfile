FROM ubuntu:20.04
# RUN mkdir /code
RUN apt-get update
RUN apt-get install make
RUN apt-get install -y gcc 
RUN apt-get install -y libpqxx-6.4


WORKDIR /code
ADD . /code
