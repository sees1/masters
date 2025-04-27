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

#########################################################################################
# rtabmap_ros

FROM rosbase AS rtabmap-ros-devel

ARG DISTRO

RUN apt-get update && apt-get install -y \
  ros-${DISTRO}-tf2 \
  ros-${DISTRO}-tf2-ros \
  ros-${DISTRO}-angles \
  ros-${DISTRO}-rtabmap \
  ros-${DISTRO}-rtabmap-ros \
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

FROM rtabmap-ros-devel AS rtabmap-ros-devel-build

FROM rtabmap-ros-devel-build AS rtabmap-ros-release

#########################################################################################
# slam_toolbox

FROM rosbase AS slam-toolbox-devel

ARG DISTRO

RUN apt-get update && apt-get install -y \
  ros-${DISTRO}-tf2 \
  ros-${DISTRO}-tf2-ros \
  ros-${DISTRO}-angles \
  ros-${DISTRO}-slam-toolbox \
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

FROM slam-toolbox-devel AS slam-toolbox-devel-build

FROM slam-toolbox-devel-build AS slam-toolbox-release

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