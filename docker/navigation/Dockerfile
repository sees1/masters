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

#########################################################################################
# nav-stack

FROM rosbase AS nav-stack-devel

ARG DISTRO

RUN apt-get update && apt-get install -y \
  ros-${DISTRO}-tf2 \
  ros-${DISTRO}-tf2-ros \
  ros-${DISTRO}-angles \
  ros-${DISTRO}-nav2-controller \
  ros-${DISTRO}-nav2-smoother \
  ros-${DISTRO}-nav2-planner \
  ros-${DISTRO}-nav2-behaviors \
  ros-${DISTRO}-nav2-bt-navigator \
  ros-${DISTRO}-nav2-waypoint-follower \
  ros-${DISTRO}-nav2-velocity-smoother \
  ros-${DISTRO}-nav2-lifecycle-manager \
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

FROM nav-stack-devel AS nav-stack-devel-build

FROM nav-stack-devel-build AS nav-stack-release