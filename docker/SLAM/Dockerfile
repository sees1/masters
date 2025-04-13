ARG UID=1000
ARG GID=1000
ARG DISTRO=humble
ARG NUM_THREADS=8
ARG CONTAINER_NAME
ARG TAG=base

from ros:humble-ros-base-jammy AS rosbase

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
