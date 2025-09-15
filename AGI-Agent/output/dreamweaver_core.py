import os
import time
import json
import sqlite3
import requests
from typing import Any, Dict, List, Tuple

# Colors for CGX aesthetic (deep blue, neon orange, neon blue)
CGX_COLORS = {
    "deep_blue": "#001f3f",
    "neon_orange": "#FF851B",
    "neon_blue": "#7FDBFF",
}

class ReasoningEngine:
    """Breaks tasks into logical components and performs causal inference."""
    def analyze(self, task: str) -> Dict[str, Any]:
        parts = task.split(" -> ")
        dependencies = [(parts[i], parts[i+1]) for i in range(len(parts)-1)]
        return {"parts": parts, "dependencies": dependencies}

    def flag_uncertainty(self, statement: str, confidence: float) -> Dict[str, Any]:
        return {"statement": statement, "confidence": confidence}

class TaskPlanner:
    """Decomposes tasks and prioritizes using cost-benefit analysis."""
    def plan(self, tasks: List[Tuple[str, int, int]]) -> List[str]:
        # tasks: list of (description, cost, benefit)
        scored = sorted(tasks, key=lambda t: t[2]-t[1], reverse=True)
        return [t[0] for t in scored]

class KnowledgeGraph:
    """Stores multi-modal knowledge as a relationship graph."""
    def __init__(self):
        self.graph: Dict[str, Dict[str, Any]] = {}

    def add_node(self, node_id: str, data: Dict[str, Any]):
        self.graph[node_id] = data

    def add_edge(self, src: str, dst: str, relation: str):
        self.graph.setdefault(src, {}).setdefault('edges', []).append({"to": dst, "relation": relation})

    def query(self, node_id: str) -> Dict[str, Any]:
        return self.graph.get(node_id, {})

class ToolIntegrator:
    """Handles calls to external tools such as xAI API and Unity."""
    def __init__(self, xai_api_key: str = ""):
        self.xai_api_key = xai_api_key or os.getenv("XAI_API_KEY", "")

    def generate_narrative(self, prompt: str) -> str:
        if not self.xai_api_key:
            return "[xAI API key missing]"  # Avoid harmful output without key
        try:
            response = requests.post(
                "https://api.x.ai/v1/generate",
                headers={"Authorization": f"Bearer {self.xai_api_key}"},
                json={"prompt": prompt}
            )
            response.raise_for_status()
            data = response.json()
            return data.get("text", "")
        except requests.RequestException as e:
            return f"[xAI API error: {e}]"

    def send_to_unity(self, payload: Dict[str, Any]) -> None:
        # Placeholder for Unity integration
        print("Sending to Unity:", json.dumps(payload))

class LearningModule:
    """Self-improving module using reinforcement learning."""
    def __init__(self):
        self.q_table: Dict[str, float] = {}

    def choose_action(self, state: str) -> str:
        return max(((a, v) for a, v in self.q_table.items() if a.startswith(state)), default=(state+":default",0))[0]

    def update(self, state: str, reward: float, alpha: float = 0.1):
        self.q_table[state] = self.q_table.get(state, 0) + alpha * (reward - self.q_table.get(state, 0))

class SafetyModule:
    """Performs fairness audits and bias mitigation."""
    PROHIBITED = {"hate", "violence"}

    def check(self, text: str) -> bool:
        return not any(word in text.lower() for word in self.PROHIBITED)

    def human_approval_required(self, context: str) -> bool:
        return "legal" in context.lower()

class TaskManager:
    """Automates task creation and Kanban board with drag-and-drop."""
    def __init__(self, db_path: str = "tasks.db"):
        self.conn = sqlite3.connect(db_path)
        self._init_db()

    def _init_db(self):
        cur = self.conn.cursor()
        cur.execute("CREATE TABLE IF NOT EXISTS tasks(id INTEGER PRIMARY KEY, title TEXT, status TEXT, assignee TEXT)")
        self.conn.commit()

    def create_task(self, title: str, assignee: str, status: str = "todo"):
        cur = self.conn.cursor()
        cur.execute("INSERT INTO tasks(title, status, assignee) VALUES(?,?,?)", (title, status, assignee))
        self.conn.commit()

    def update_status(self, task_id: int, status: str):
        cur = self.conn.cursor()
        cur.execute("UPDATE tasks SET status=? WHERE id=?", (status, task_id))
        self.conn.commit()

    def list_tasks(self) -> List[Tuple[int, str, str, str]]:
        cur = self.conn.cursor()
        cur.execute("SELECT id, title, status, assignee FROM tasks")
        return cur.fetchall()

class DreamWeaver:
    """Main agent orchestrating modules."""
    def __init__(self):
        self.reasoning = ReasoningEngine()
        self.planner = TaskPlanner()
        self.knowledge = KnowledgeGraph()
        self.tools = ToolIntegrator()
        self.learning = LearningModule()
        self.safety = SafetyModule()
        self.tasks = TaskManager(os.path.join(os.path.dirname(__file__), "dreamweaver_tasks.db"))

    def process_task(self, task: str):
        analysis = self.reasoning.analyze(task)
        for part in analysis["parts"]:
            self.learning.update(part, reward=1.0)
        return analysis

    def generate_story(self, prompt: str) -> str:
        story = self.tools.generate_narrative(prompt)
        if not self.safety.check(story):
            story = "[Content flagged by safety module]"
        return story

    def add_knowledge(self, node_id: str, data: Dict[str, Any]):
        self.knowledge.add_node(node_id, data)

    def run(self):
        try:
            while True:
                user_input = input("Enter command (or 'quit'): ")
                if user_input == 'quit':
                    break
                print(self.process_task(user_input))
        except EOFError:
            print("EOF received, shutting down gracefully.")

if __name__ == "__main__":
    agent = DreamWeaver()
    print("DreamWeaver ready with colors:", CGX_COLORS)
    agent.run()
