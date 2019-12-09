#!/usr/bin/env python3

import sys
import string

class Task(object):
    def __init__(self, name):
        self.name = name
        self.assigned = False
        self.completed = False
        self.depends = []

    def pos(self):
        return string.ascii_uppercase.index(self.name) - string.ascii_uppercase.index('A')

class TaskPool(object):
    def __init__(self):
        self.tasks = {}

    def add_rule(self, taskname, dependson):
        task = self.tasks.get(taskname)
        if not task:
            task = Task(taskname)
            self.tasks[taskname] = task

        dep = self.tasks.get(dependson)
        if not dep:
            dep = Task(dependson)
            self.tasks[dependson] = dep

        task.depends.append(dep)

    def find_available(self):
        avail = None
        for task in self.tasks.values():
            if task.assigned or task.completed:
                continue

            found = True
            for d in task.depends:
                if not d.completed:
                    found = False
                    break

            if found and (avail is None or task.name < avail.name):
                avail = task

        return avail


class Worker(object):
    def __init__(self):
        self.assigned_task = None
        self.end = 0

class WorkPool(object):
    def __init__(self, size):
        self.workers = [Worker() for i in range(size)]
        self.busy_workers = 0
        self.tasks_done = []

    def get_worker(self):
        for w in self.workers:
            if w.assigned_task is None:
                return w
        return None

    def assign_task(self, task, end):
        w = self.get_worker()
        if w:
            task.assigned = True
            w.assigned_task = task
            w.end = end
            self.busy_workers += 1
            return True

        return False

    def find_next_end(self):
        end = 10000000000
        for w in self.workers:
            if w.assigned_task and w.end < end:
                end = w.end
        return end

    def process(self, now):
        for w in self.workers:
            if w.assigned_task and w.end == now:
                self.tasks_done.append(w.assigned_task)
                self.busy_workers -= 1
                w.assigned_task.completed = True
                w.assigned_task = None

    def print(self, now):
        values = []
        values.append("{:4} ".format(now))

        for w in self.workers:
            if w.assigned_task:
                values.append(" {} ".format(w.assigned_task.name))
            else:
                values.append(" . ")

        for t in self.tasks_done:
            values.append("{}".format(t.name))

        print("".join(values))

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: {} <filename> <numworkers>".format(sys.argv[0]), file = sys.stderr)
        sys.exit(-1)

    try:
        wsize = int(sys.argv[2])
    except:
        print("Cannot parse the number of workers: {}".format(sys.argv[2]), file = sys.stderr)
        sys.exit(-1)

    wp = WorkPool(wsize)
    tp = TaskPool()
    try:
        with open(sys.argv[1], "r") as f:
            for line in f:
                a = line.split(" ")
                required, task = a[1], a[7]
                tp.add_rule(task, required)
    except:
        print("Cannot open {} for reading".format(sys.argv[1]), file = sys.stderr)
        sys.exit(-1)

    now = 0
    while True:
        while True:
            task = tp.find_available()
            if task is None:
                break

            end = now + task.pos() + 61
            if not wp.assign_task(task, end):
                break

        wp.print(now)

        if wp.busy_workers == 0:
            break

        now = wp.find_next_end()
        wp.process(now)

    print("Seconds: {}".format(now))
