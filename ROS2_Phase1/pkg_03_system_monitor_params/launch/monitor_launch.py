from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
import os

def generate_launch_description():
    config = os.path.join(
        get_package_share_directory('pkg_03_system_monitor_params'),
        'config',
        'monitor_params.yaml'
    )

    return LaunchDescription([
        Node(
            package='pkg_03_system_monitor_params',
            executable='monitor_node',
            name='system_monitor',
            parameters=[config],
            output='screen'
        )
    ])

