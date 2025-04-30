import os

from ament_index_python.packages import get_package_share_directory

from launch import LaunchDescription
from launch.actions import ExecuteProcess
from launch_ros.actions import Node

def generate_launch_description():
  current_path = '~'

  # bags_path = os.path.join(get_package_share_directory(current_path), 'bags')
  amcl_param_path = os.path.join(get_package_share_directory(current_path), 'config', 'amcl.yaml')

  # rosbag_provider = [
  #   'ros2',
  #   'bag',
  #   'play',
  #   '-l',
  #   bags_path
  # ]
  
  # rosbag_play = ExecuteProcess(cmd = rosbag_provider, output='screen')

  amcl = Node(
      package="nav2_amcl",
      executable='amcl',
      output='screen',
      parameters=[amcl_param_path]
  )

  ld = LaunchDescription()
  # ld.add_action(rosbag_play)
  ld.add_action(amcl)
  return ld