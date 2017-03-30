FROM ruby:2.3

RUN git clone git://github.com/AboutUs/pegleg.git /usr/local/src/pegleg \
 && cd /usr/local/src/pegleg \
 && make \
 && make install \
 && apt-get update \
 && apt-get install -y --no-install-recommends graphviz time gdb
RUN useradd \
 --uid 1000 --gid 50 \
 --shell /bin/bash \
 --create-home --home-dir /exploratory-parsing \
 web
RUN gem install sinatra \
 && gem install haml \
 && gem install diffy
COPY . /usr/local/src/exploratory-parsing
WORKDIR /exploratory-parsing
USER web
RUN cp -R /usr/local/src/exploratory-parsing/* /exploratory-parsing/
EXPOSE 8080
CMD ["rackup", "--host", "0.0.0.0", "-p", "8080"]
