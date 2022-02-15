import sys

from cellworld_experiment_service import ExperimentClient

subject_name = sys.argv[1]
occlusions = sys.argv[2]

client = ExperimentClient()
client.connect()
print(client.start_experiment(prefix="HUMAN", suffix="TEST", subject_name=subject_name, world_configuration="hexagonal", world_implementation="vr", occlusions=occlusions, duration=10))
