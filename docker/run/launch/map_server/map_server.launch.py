import os

from ament_index_python.packages import get_package_share_directory

from launch import LaunchDescription
from launch.actions import EmitEvent, RegisterEventHandler
from launch_ros.actions import Node
from launch_ros.actions import LifecycleNode
from launch_ros.events.lifecycle import ChangeState, matches_node_name
from launch.events import matches_action
from lifecycle_msgs.msg import Transition
from launch_ros.event_handlers import OnStateTransition



def generate_launch_description():
  home_dir = os.path.expanduser('~')
  current_path = home_dir + '/config'

  server_param_path = os.path.join(current_path, 'map.yaml')

  map_server_node = LifecycleNode(
        package='nav2_map_server',
        executable='map_server',
        name='map_server',
        namespace='',
        output='screen',
        parameters=[{'yaml_filename': server_param_path}]
  )

  # 2) Как только map_server окажется в 'unconfigured', шлём ему TRANSITION_CONFIGURE
  configure_handler = RegisterEventHandler(
      event_handler=OnStateTransition(
          target_lifecycle_node=map_server_node,
          goal_state='unconfigured',
          entities=[
              EmitEvent(
                  event=ChangeState(
                      lifecycle_node_matcher=matches_action(map_server_node),
                      transition_id=Transition.TRANSITION_CONFIGURE,
                  )
              )
          ],
      )
  )

  # 3) Как только map_server окажется в 'inactive', шлём ему TRANSITION_ACTIVATE
  activate_handler = RegisterEventHandler(
      event_handler=OnStateTransition(
          target_lifecycle_node=map_server_node,
          goal_state='inactive',
          entities=[
              EmitEvent(
                  event=ChangeState(
                      lifecycle_node_matcher=matches_action(map_server_node),
                      transition_id=Transition.TRANSITION_ACTIVATE,
                  )
              )
          ],
      )
  )

      # 3) Наконец, по завершении запуска лаунча сразу шлём TRANSITION_CONFIGURE
  configure_immediately = EmitEvent(
      event=ChangeState(
          lifecycle_node_matcher=matches_action(map_server_node),
          transition_id=Transition.TRANSITION_CONFIGURE,
      )
  )


  ld = LaunchDescription()
  ld.add_action(map_server_node)
  ld.add_action(configure_handler)
  ld.add_action(activate_handler)
  ld.add_action(configure_immediately)
  return ld
