import std.conv;
import std.format;
import std.stdio;

struct Task {
	char name;
	bool assigned;
	bool completed;
	Task*[] depends;

	this(char name) {
		this.name = name;
		this.assigned = false;
		this.completed = false;
	}
}

struct TaskPool {
	Task*[int] tasks;

	void addRule(char taskname, char dependson) {
		Task *task;
		if (taskname in this.tasks) {
			task = this.tasks[taskname];
		} else {
			task = new Task(taskname);
			this.tasks[taskname] = task;
		}

		Task *dep;
		if (dependson in this.tasks) {
			dep = this.tasks[dependson];
		} else {
			dep = new Task(dependson);
			this.tasks[dependson] = dep;
		}
		task.depends ~= dep;
	}

	Task *findAvailable() {
		Task *avail = null;
		foreach (task; this.tasks.byValue()) {
			if (task.assigned || task.completed) {
				continue;
			}

			bool found = true;
			foreach (d; task.depends) {
				if (!d.completed) {
					found = false;
					break;
				}
			}

			if (found && (avail == null || task.name < avail.name)) {
				avail = task;
			}
		}
		return avail;
	}
}

struct Worker {
	Task *assignedTask;
	int end;
}

struct WorkPool {
	Worker*[] workers;
	int busyWorkers;
	Task*[] tasksDone;

	this(int workers) {
		for (int i = 0; i < workers; ++i) {
			this.workers ~= new Worker;
		}
		this.busyWorkers = 0;
	}

	Worker *getWorker() {
		foreach (w; workers) {
			if (w.assignedTask == null) {
				return w;
			}
		}
		return null;
	}

	bool assignTask(Task *task, int end) {
		Worker *w = this.getWorker();
		if (w != null) {
			++this.busyWorkers;
			w.assignedTask = task;
			w.end = end;
			task.assigned = true;
			return true;
		}
		return false;
	}

	int findNextEnd() {
		int end = int.max;
		foreach (w; this.workers) {
			if (w.assignedTask != null && w.end < end) {
				end = w.end;
			}
		}
		return end;
	}

	void process(int now) {
		foreach (w; this.workers) {
			if (w.assignedTask != null && w.end == now) {
				this.tasksDone ~= w.assignedTask;
				--this.busyWorkers;
				w.assignedTask.completed = true;
				w.assignedTask = null;
			}
		}
	}

	void print(int now) {
		writef("%4d", now);
		foreach(w; this.workers) {
			if (w.assignedTask == null) {
				write(" . ");
			} else {
				writef(" %c ", w.assignedTask.name);
			}
		}
		foreach(t; this.tasksDone) {
			writef("%c", t.name);
		}
		writeln();
	}
}

int main(string[] args) {
	if (args.length < 3) {
		stderr.writef("Usage: %s <filename> <numworkers>\n", args[0]);
		return -1;
	}

	int wsize;
	try {
		wsize = to!int(args[2]);
	} catch (std.conv.ConvException) {
		stderr.writef("Cannot parse the number of workers: %s\n", args[2]);
		return -1;
	}

	File input;
	try {
		input = File(args[1], "r");
	} catch (std.exception.ErrnoException) {
		stderr.writef("Cannot open %s for reading\n", args[1]);
		return -1;
	}

	TaskPool tp = TaskPool();
	WorkPool wp = WorkPool(wsize);

	char requisite, taskname;
	try {
		while(input.readf("Step %c must be finished before step %c can begin.\n",
				  &requisite, &taskname)) {
			tp.addRule(taskname, requisite);
		}
	} catch (FormatException) {
	} finally {
		input.close();
	}

	int now = 0;
	while (true) {
		while (true) {
			Task *task = tp.findAvailable();
			if (task == null) {
				break;
			}
			int end = now + task.name - 'A' + 61;
			if (!wp.assignTask(task, end)) {
				break;
			}
		}
		wp.print(now);

		if (wp.busyWorkers == 0) {
			break;
		}

		now = wp.findNextEnd();
		wp.process(now);
	}

	writeln("Seconds: ", now);

	return 0;
}
