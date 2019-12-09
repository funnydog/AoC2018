package main

import (
	"fmt"
	"os"
	"strconv"
)

const MaxInt = int(^uint(0) >> 1)

type Task struct {
	Name      int
	Assigned  bool
	Completed bool
	Depends   []*Task
}

type TaskPool struct {
	Tasks map[int]*Task
}

func (tp *TaskPool) AddRule(taskName, dependsOn int) {
	task, ok := tp.Tasks[taskName]
	if !ok {
		task = &Task{Name: taskName}
		tp.Tasks[taskName] = task
	}

	dep, ok := tp.Tasks[dependsOn]
	if !ok {
		dep = &Task{Name: dependsOn}
		tp.Tasks[dependsOn] = dep
	}
	task.Depends = append(task.Depends, dep)
}

func (tp *TaskPool) FindAvailable() *Task {
	var avail *Task
	for _, task := range tp.Tasks {
		if task.Assigned || task.Completed {
			continue
		}

		found := true
		for _, dep := range task.Depends {
			if !dep.Completed {
				found = false
			}
		}

		if found && (avail == nil || task.Name < avail.Name) {
			avail = task
		}
	}
	return avail
}

func makeTaskPool() TaskPool {
	return TaskPool{Tasks: map[int]*Task{}}
}

type Worker struct {
	Name         int
	AssignedTask *Task
	End          int
}

type WorkPool struct {
	Workers     []*Worker
	BusyWorkers int
	TasksDone   []*Task
}

func (wp *WorkPool) GetWorker() *Worker {
	for _, w := range wp.Workers {
		if w.AssignedTask == nil {
			return w
		}
	}
	return nil
}

func (wp *WorkPool) AssignTask(task *Task, end int) bool {
	w := wp.GetWorker()
	if w != nil {
		wp.BusyWorkers++
		w.AssignedTask = task
		w.End = end
		task.Assigned = true
		return true
	}
	return false
}

func (wp *WorkPool) FindMinEnd() int {
	end := MaxInt
	for _, w := range wp.Workers {
		if w.AssignedTask != nil && w.End < end {
			end = w.End
		}
	}
	return end
}

func (wp *WorkPool) Process(now int) {
	for _, w := range wp.Workers {
		if w.AssignedTask != nil && w.End == now {
			wp.TasksDone = append(wp.TasksDone, w.AssignedTask)
			wp.BusyWorkers--
			w.AssignedTask.Completed = true
			w.AssignedTask = nil
		}
	}
}

func (wp *WorkPool) Print(now int) {
	fmt.Printf("%4d", now)
	for _, w := range wp.Workers {
		if w.AssignedTask != nil {
			fmt.Printf(" %c ", w.AssignedTask.Name)
		} else {
			fmt.Printf(" . ")
		}
	}
	for _, t := range wp.TasksDone {
		fmt.Printf("%c", t.Name)
	}
	fmt.Println()
}

func makeWorkPool(size int) WorkPool {
	workers := []*Worker{}
	for i := 0; i < size; i++ {
		workers = append(workers, &Worker{
			Name:         i,
			AssignedTask: nil,
			End:          0,
		})
	}
	return WorkPool{
		Workers:   workers,
		TasksDone: []*Task{},
	}
}

func main() {
	val := mainWithInt()
	if val != 0 {
		os.Exit(val)
	}
}

func mainWithInt() int {
	if len(os.Args) < 3 {
		fmt.Fprintf(os.Stderr, "Usage %s [filename] [numworkers]\n", os.Args[0])
		return -1
	}

	wsize, err := strconv.ParseInt(os.Args[2], 10, 64)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Cannot parse the number of workers\n")
		return -1
	}

	input, err := os.Open(os.Args[1])
	if err != nil {
		fmt.Fprintf(os.Stderr, "Cannot open %s for reading\n", os.Args[1])
		return -1
	}
	defer input.Close()

	tp := makeTaskPool()
	wp := makeWorkPool(int(wsize))

	var requisite, task int
	for {
		_, err := fmt.Fscanf(input, "Step %c must be finished before step %c can begin.\n",
			&requisite, &task)
		if err != nil {
			break
		}

		tp.AddRule(task, requisite)
	}

	now := 0
	for {
		for {
			task := tp.FindAvailable()
			if task == nil {
				break
			}
			end := now + task.Name - 'A' + 61
			if !wp.AssignTask(task, end) {
				break
			}
		}

		wp.Print(now)

		if wp.BusyWorkers == 0 {
			break
		}

		now = wp.FindMinEnd()
		wp.Process(now)
	}

	fmt.Println("Seconds:", now)

	return 0
}
