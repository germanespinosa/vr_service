from time import sleep
import sys
from random import choice
from cellworld import World, Display, Location, Agent_markers, Capture, Capture_parameters, Step, Timer, Cell_group_builder, to_radians, to_degrees, Location_list, Cell_group
from cellworld_controller_service import ControllerClient
from cellworld_experiment_service import ExperimentClient
from matplotlib.backend_bases import MouseButton
from cellworld_tracking import TrackingClient

class AgentData:
    def __init__(self, agent_name: str):
        self.is_valid = None
        self.step = Step()
        self.step.agent_name = agent_name


display = None
world = None
occlusions = Cell_group_builder()


def on_experiment_started(experiment):
    print("Experiment started:", experiment)
    experiments[experiment.experiment_name] = experiment.copy()


episode_in_progress = False

free_cells = Cell_group()

def on_episode_started(experiment_name):
    global occlusions
    global episode_in_progress
    global free_cells
    print("New Episode!!!", experiment_name)
    occlusions = Cell_group_builder.get_from_name("hexagonal", experiments[experiment_name].world.occlusions, "occlusions")
    world.set_occlusions(occlusions)
    free_cells = world.cells.free_cells()
    episode_in_progress = True


def on_capture(frame: int):
    print("you've been captured!!")


def on_episode_finished():
    global episode_in_progress
    episode_in_progress = False
    print("episode finished")


experiment_service = ExperimentClient()
experiment_service.on_experiment_started = on_experiment_started
experiment_service.on_episode_started = on_episode_started
experiment_service.on_episode_finished = on_episode_finished
experiment_service.on_capture = on_capture

if not experiment_service.connect():
    print("Failed to connect to experiment service")
    exit(1)
experiment_service.subscribe()


tracking_client = TrackingClient()
if not tracking_client.connect():
    print("Failed to connect to agent_tracking service")
    exit(1)
tracking_client.set_throughput(5)
tracking_client.subscribe()


experiments = {}

predator = AgentData("predator")
prey = AgentData("prey")

behavior = -1
time_out = 1.0


def on_step(step: Step):
    if step.agent_name == "predator":
        predator.is_valid = Timer(time_out)
        predator.step = step
    else:
        prey.is_valid = Timer(time_out)
        prey.step = step

# connect to controller

controller = ControllerClient()

if not controller.connect("127.0.0.1", 4590):
    print("failed to connect to the controller")
    exit(1)

controller.subscribe()
controller.on_step = on_step


world = World.get_from_parameters_names("hexagonal", "canonical")

display = Display(world, fig_size=(9.0*.75, 8.0*.75), animated=True)


last_destination_update = None
last_destination_sent = None


def set_destination(destination: Location):
    global last_destination_update
    global controller
    global last_destination_sent
    last_destination_update = Timer(2)
    controller.set_destination(destination)
    last_destination_sent = destination


def check_destination_timeout():
    global last_destination_update
    if last_destination_sent and last_destination_update.time_out():
        last_destination_update = Timer(2)
        print(last_destination_sent)
        controller.set_destination(last_destination_sent)


display.set_agent_marker("predator", Agent_markers.arrow())
display.set_agent_marker("prey", Agent_markers.arrow())

local_prey_step = Step(agent_name="prey")
experiment = experiment_service.start_experiment("TEST", "TEST", "hexagonal", "canonical", "10_02", "TEST", 2)


last_chasing_destination_timer = Timer(0)
last_exploring_destination_timer = Timer(0)
last_exploring_destination = Location()


def reached(position: Location, destination:Location):
    return position.dist(destination) < world.implementation.cell_transformation.size


while True:

    while not episode_in_progress: #wait for the episode to start
        display.update()

    while episode_in_progress: #run until episode is finished
        if prey.is_valid:
            display.agent(step=prey.step, color="red", size=20)
            if last_chasing_destination_timer.time_out(): #it can see the prey so let's go towards it
                set_destination(prey.step.location)
                last_chasing_destination_timer = Timer(.5)
        else:
            if "prey" in tracking_client.current_states:
                display.agent(step=tracking_client.get_current_state("prey"), color="green", size=15)
            if last_exploring_destination_timer.time_out() or reached(predator.step.location, last_exploring_destination):
                display.circle(last_exploring_destination, .01, "white")
                last_exploring_destination = choice(free_cells).location
                set_destination(last_exploring_destination)
                last_exploring_destination_timer = Timer(10)
                display.circle(last_exploring_destination, .01, "blue")
        check_destination_timeout()

        if predator.is_valid:
            display.agent(step=predator.step, color="blue", size=15)
        else:
            display.agent(step=predator.step, color="gray", size=15)
        display.update()

