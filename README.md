# IHTP Solver

This repository contains a solver for the **Integrated Healthcare Timetabling Problem (IHTP)** implemented using the **EasyLocal framework**. 

The solver is based on a multi-neighborhood Simulated Annealing metaheuristic tailored to the structure of the IHTP. It was developed for research purposes to evaluate performance against the solutions submitted to the **Integrated Healthcare Timetabling Competition 2024 (IHTC-2024)**.

The algorithmic approach and experimental results are described in the paper:

> **A Multi-Neighborhood Simulated Annealing Approach to the Integrated Healthcare Timetabling Problem**
> *(currently under review)*

## The Problem

The IHTP models the path of a patient through a surgical ward.

A solution to the problem requires to determine for each patient:

- The day they will be admitted (may be NULL if the patient is optional);
- The room they are assigned to;
- The operating theater they are assigned to;
- The on-duty nurse assigned on-duty nurse is assigned to each non-empty room for every shift in the planning horizon.

For a description of the instance and output file formats we refer to: https://ihtc2024.github.io

## Setup

Clone the repository with submodules:

```bash
git clone --recurse-submodules <repo-url>
```

Build the solver:

```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j
```


## Running the Solver

Two versions of the algorithm are provided:

* **Time-based version** – used to obtain the results reported in the paper.
* **Iteration-based version** – fully deterministic when the random seed is specified, allowing exact reproducibility.

### Time-based version

To run the time-based version of the solver:

```bash
./ih \
  --main::instance <instance_file_path> \
  --main::method PSA_TB \
  --IH_TB_PSA::cooling_rate 0.88 \
  --IH_TB_PSA::start_temperature 842.2 \
  --IH_TB_PSA::min_temperature 0.248 \
  --IH_TB_PSA::neighbors_accepted_ratio 0.17 \
  --IH_TB_PSA::allowed_running_time 600 \
  --IH_TB_PSA::max_evaluations 550000000 \
  [ --main::output_file <output_file_path> ]
```

This configuration corresponds to the one used to generate the experimental results reported in the paper.

### Iteration-based version

To run the deterministic (iteration-based) version:

```bash
./ih \
  --main::instance <instance_file_path> \
  --main::method PSA \
  --main::seed <seed> \
  --IH_PSA::cooling_rate 0.88 \
  --IH_PSA::start_temperature 842.2 \
  --IH_PSA::min_temperature 0.248 \
  --IH_PSA::neighbors_accepted_ratio 0.17 \
  --IH_PSA::max_evaluations <num_evaluations> \
  [ --main::output_file <output_file_path> ]
```

Providing a **seed value** ensures that runs are fully reproducible.




