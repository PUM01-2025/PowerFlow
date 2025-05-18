import random
import typing


def generate_grid(file: typing.TextIO, grid_no: int) -> tuple[int, int]:

    network = {}
    grid_size = random.randint(20, 200)
    file.write("grid\n")
    file.write(f"# {grid_no} \n")
    file.write("10000000 400\n")

    for i in range(1, grid_size):
        parent = random.randint(0, i - 1)
        if parent in network:
            network[parent].append(i)
        else:
            network[parent] = [i]

    for key in network:
        for value in network[key]:
            file.write(f"{key} {value} (0.05, 0.05)\n")

    file.write("%\n")

    file.write("0 s\n")

    load_nodes = 0

    for value in network.values():
        for node in value:
            if node not in network:
                file.write(f"{node} l\n")
                load_nodes += 1

    file.write("%\n")
    return (grid_size, load_nodes)


def generate_network(file_name: str, loops: bool = False):
    with open(file_name, "w") as f:
        networks = random.randint(2000, 4000)
        size = 1
        loads = 0
        for i in range(0, networks):
            (grid_size, grid_loads) = generate_grid(f, i)
            size += grid_size + 1
            loads += grid_loads

        f.write("grid\n")
        f.write("1000000000 10000\n")
        for i in range(1, networks+1):
            f.write(f"{i} {i-1} (0.0005, 0.0005)\n")
        f.write(f"{0} {i} (0.0005, 0.0005)\n")

        f.write("%\n")
        f.write("0 e\n")
        f.write("%\n")
        f.write("connections\n")
        for i in range(1, networks+1):
            f.write(f"{networks} {i} {i-1} 0\n")

        f.write("%\n")

        print(f"Generated a network with {networks} subnets, {size} nodes and {loads} loads")


if __name__ == "__main__":
    generate_network("stort_natverk.txt")

    # generate_grid(1)
