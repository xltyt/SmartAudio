#
# docker build -t cosyvoice_base:0.1 -f Dockerfile .
#
FROM ubuntu:22.04 as build
RUN ln -sf /usr/share/zoneinfo/Asia/Shanghai /etc/localtime && echo 'Asia/Shanghai' > /etc/timezone
RUN apt update
RUN apt install -y vim lrzsz net-tools lsb-core wget sysstat psmisc curl unzip zip
RUN apt install -y build-essential gcc bison make git pip python3 python-is-python3
COPY requirements.txt /
RUN pip install -r /requirements.txt -i https://pypi.tuna.tsinghua.edu.cn/simple
RUN rm -f /requirements.txt
RUN apt-get clean
RUN pip cache purge
FROM ubuntu:22.04
COPY --from=build / /
CMD ["/bin/bash"]
