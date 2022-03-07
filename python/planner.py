from cellworld import Step, Location, World, Cell_group, Cell_group_builder, World_implementation, Timer
from cellworld_controller_service import ControllerClient
from cellworld_experiment_service import ExperimentClient, StartExperimentResponse
from random import choice

Stopped = 0
Paused = 1
Running = 2

state = Paused

world = World.get_from_parameters_names("hexagonal", "canonical")

destination = Location()
predator_location = Location()
distance_threshold = .05


def on_step(step: Step):
    global destination
    global predator_location
    if step.agent_name == "prey":
        destination = step.location
        experiment_client.set_behavior(1)#chasing
    else:
        predator_location = step.location


experiment_worlds = {}


def on_experiment_started(experiment: StartExperimentResponse):
    global experiment_worlds
    experiment_worlds[experiment.experiment_name] = experiment.world


possible_destinations = []


def on_episode_started(experiment_name: str):
    global possible_destinations
    occlusions = Cell_group_builder.get_from_name("hexagonal", experiment_worlds[experiment_name], "occlusions")
    world.set_occlusions(occlusions)
    free_cells = world.cells.free_cells()
    new_possible_destinations = []
    for cell in free_cells:
        new_possible_destinations.append(cell.location)
    possible_destinations = new_possible_destinations
    state = Running


def on_episode_finished():
    global state
    state = Paused


controller_client = ControllerClient()
controller_client.on_step = on_step

if not controller_client.connect():
    print("Failed to connect to controller")
    exit(1)

experiment_client = ExperimentClient()
experiment_client.on_experiment_started = on_experiment_started
experiment_client.on_episode_started = on_episode_started
experiment_client.on_episode_finished = on_episode_finished

if not experiment_client.connect():
    print("Failed to connect to experiment")
    exit(1)

last_destination_timer = Timer(2)

while state != Stopped:
    if predator_location.dist(destination) < distance_threshold:
        experiment_client.set_behavior(0) #exploring
        destination = choice(possible_destinations)

    if last_destination_timer.time_out():
        last_destination_timer.reset()
        if destination:
            controller_client.set_destination(destination)


