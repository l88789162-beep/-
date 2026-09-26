# agent_scheduler.py 九智能体稽查流水线入口
import asyncio
from dataclasses import dataclass
from typing import List,Dict
import numpy as np

@dataclass
class TaskItem:
    task_id:str
    lagrangian:str
    priority:int

class NineAgentOrchestrator:
    def __init__(self):
        self.kb = dict()
        self.eb = dict()
        self.mb = dict()
        self.task_queue:asyncio.Queue[TaskItem] = asyncio.Queue()

    async def submit_task(self,task:TaskItem):
        await self.task_queue.put(task)

    async def worker(self,agent_id:int):
        while True:
            task = await self.task_queue.get()
            print(f"Agent{agent_id} start task:{task.task_id}, L={task.lagrangian}")
            score = await self.run_full_audit(task.lagrangian)
            print(f"Task {task.task_id} finished, model score={score}")
            self.task_queue.task_done()

    async def run_full_audit(self,lag:str)->float:
        from tensor_parser import TensorParser
        tp = TensorParser()
        parse_result = tp.parse_lagrangian(lag)
        if not parse_result.valid:
            return -1.0
        dim_check = tp.check_dimension(parse_result.terms)
        if not dim_check.pass_flag:
            return -2.0
        eq_result = tp.derive_eom(parse_result)
        stab_score = tp.stability_analysis(eq_result)
        power_count = tp.eft_power_count(parse_result)
        final_score = stab_score * 0.6 + power_count *0.4
        return final_score

    async def start(self,worker_count=4):
        workers = [asyncio.create_task(self.worker(i)) for i in range(worker_count)]
        await self.task_queue.join()
        for w in workers:
            w.cancel()

async def main():
    engine = NineAgentOrchestrator()
    test_tasks = [
        TaskItem("task01","L = 1/2 g^{μν}∂_μφ∂_νφ - V(φ)",priority=1),
        TaskItem("task02","L = R + c1 R^2 + c2 R_{μν}R^{μν}",priority=2)
    ]
    for t in test_tasks:
        await engine.submit_task(t)
    await engine.start(worker_count=3)

if __name__=="__main__":
    asyncio.run(main())
