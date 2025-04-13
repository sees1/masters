ARG UID=1000
ARG GID=1000
ARG DISTRO=humble
ARG NUM_THREADS=8
ARG CONTAINER_NAME
ARG TAG=base

from ros:humble-ros-base-jammy AS rosbase

#########################################################################################
# map-server

FROM rosbase AS map-server-devel

ARG DISTRO

RUN apt-get update && apt-get install -y \
  ros-${DISTRO}-tf2 \
  ros-${DISTRO}-tf2-ros \
  ros-${DISTRO}-angles \
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

FROM map-server-devel AS map-server-devel-build

FROM map-server-devel-build AS map-server-release