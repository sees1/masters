ARG UID=1000
ARG GID=1000
ARG DISTRO=humble
ARG NUM_THREADS=8
ARG CONTAINER_NAME
ARG TAG=base

from ros:humble-ros-base-jammy AS rosbase

#########################################################################################
# amcl

FROM rosbase AS amcl-devel

ARG DISTRO

RUN apt-get update && apt-get install -y \
  ros-${DISTRO}-tf2 \
  ros-${DISTRO}-tf2-ros \
  ros-${DISTRO}-angles \
  ros-${DISTRO}-nav2-amcl \
  ros-${DISTRO}-nav2-map-server \
  && rm -rf /var/lib/apt/lists/*

ARG CONTAINER_NAME
ARG USER=docker_${CONTAINER_NAME}
ARG UID
ARG GID
ARG PW=user

# Create a non-root user
RUN groupadd --gid $GID $USER \
  && useradd -s /bin/bash --uid $UID --gid $GID -m $USER \
  # Add sudo support for the non-root user
  && apt-get update \
  && apt-get install -y sudo \
  && echo $USER ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USER\
  && chmod 0440 /etc/sudoers.d/$USER \
  && rm -rf /var/lib/apt/lists/*

WORKDIR /home/$USER
RUN cd /home/$USER && mkdir -p colcon_ws/src colcon_ws/log colcon_ws/install && chown -R ${UID}:${GID} ./
RUN usermod -G dialout -a $USER && newgrp dialout

FROM amcl-devel AS amcl-devel-build

FROM amcl-devel-build AS amcl-release