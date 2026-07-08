from setuptools import setup, find_packages

package_name = 'path_obstacle_detector'

setup(
    name=package_name,
    version='0.1.0',
    packages=find_packages(),
    data_files=[
        ('share/ament_index/resource_index/packages', ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='your_name',
    maintainer_email='you@example.com',
    description='基于全局路径的激光雷达障碍物检测',
    license='Apache-2.0',
    entry_points={
        'console_scripts': [
            'path_obstacle_detector = path_obstacle_detector.path_obstacle_detector_node:main',
            'test_publisher       = path_obstacle_detector.test_publisher:main',
        ],
    },
)