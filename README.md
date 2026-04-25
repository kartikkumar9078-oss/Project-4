The goal of this project is to save lives by streamlining the emergency response pipeline in India. 
The system filters incoming emergency alerts to eliminate false alarms and identifies the most suitable vehicle for dispatch. 
By integrating advanced pathfinding algorithms, we ensure that emergency vehicles navigate the most efficient routes to reach victims in the shortest possible time.

The system is modularized into Preprocessing, Alert Analysis, Vehicle Selection, and Route Optimization.
•	Alert Filtering: Uses Aho-Corasick, KMP, and Rabin-Karp for high-speed string matching to verify emergency signals.
•	Route Optimization: We have transitioned from basic Dijkstra to the A* algorithm for faster, heuristic-based pathfinding.
•	Reliability: To handle road blockages, we have implemented Yen’s algorithm to precompute $k$-shortest alternative paths.
•	Frontend: A Map API is being integrated to provide a real-time visual interface for vehicle tracking and navigation.
•	Tech Stack: C++ for core logic, Python for background logic and GUI components, and VSCode for development.

Through this project, we were able to develop a working model of an emergency alert and vehicle dispatch system that focuses on improving response time during critical situations. 
The system can take an emergency input, process it, and identify the most suitable vehicle to respond using simple and efficient logic.
We also implemented path-finding techniques to ensure that the selected vehicle reaches the location using the best possible route.
Along with this, we explored different algorithms for handling and analyzing input data, which helped us understand how real-world systems manage large amounts of information quickly.
The project is divided into different modules, making it easy to understand, test, and improve further.
Overall, this project helped us gain practical knowledge of applying algorithms to solve real-life problems.
It also provides a strong base that can be expanded in the future by adding features like live tracking, better user interface, and real-time communication.
