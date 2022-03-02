from cellworld_experiment_service import ExperimentClient

client = ExperimentClient()
client.connect()
print(client.set_behavior(1))

