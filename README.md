Made with Unreal Engine Version: 5.5.4-40574608+++UE5+Release-5.5

---------------------------------
  How to Use the Simulation
-----------------------------------

- Use **W, A, S, D** keys to move the camera.
- Use the mouse to look around the environment.
- Observe a number of agents (defined in the SimulationDriver) battling on a grid of predefined size.

Simulation Details:
• The simulation is deterministic — all agent decisions (movement and attack) are based on fixed logic.
• The random seed is hardcoded for testing purposes.
• Because of this, the Blue team is expected to win every time in the default scenario, and the final agent positions should remain identical across runs.



--------------------------------------
  Custom Simulation Setup Instructions
--------------------------------------

To configure your own simulation setup:

1. **Set the Number of Agents per Team:**
   - Go to: `Content/MySimulationDriver`
   - Modify the `NumAgentsPerTeam` property to your desired value.

2. **Switch Between Grid Types (Square / Hex):**
   - Still in `Content/MySimulationDriver`, set the `GridConfig` property to one of:
     • `DA_SquareGridConfig` — for a square grid
     • `DA_HexGridConfig`    — for a hexagonal grid

3. **Adjust Grid Size:**
   - Open the chosen grid config asset (`DA_SquareGridConfig` or `DA_HexGridConfig`)
   - Change the `GridSize` field to your desired map dimensions.



Note: Given more time, I would focus on refining the code structure and looking into performance improvements — 
such as optimizing pathfinding by allowing agents to share paths when appropriate.
