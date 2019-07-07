/**
* @file d_star_lite.cpp
* @author vss2sn
* @brief Contains the DStarLite class
*/

#include "d_star_lite.hpp"

void DStarLite::VectorInsertionSort(std::vector<Node>& v){
   int nV = v.size();
   int i, j;
   Node key;
   for (i = 1; i < nV; i++) {
       key = v[i];
       j = i-1;
       while (j >= 0 && (v[j].cost_ + v[j].h_cost_ >= key.cost_+key.h_cost_)){
           v[j+1] = v[j];
           j--;
       }
       v[j+1] = key;
   }
}

double DStarLite::GetHeuristic(Node s1, Node s2){
  return abs(s1.x_ - s2.x_) + abs(s1.y_ - s2.y_);
}

void DStarLite::PrintGRHS(){
  std::cout << "G values:" << std::endl;
  for(int i=0;i<n;i++){
    for(int j=0;j<n;j++){
      std::cout<< std::setw(5) <<S_[i][j].first << ",";
    }
    std::cout << std::endl;
  }
  std::cout << "RHS values:" << std::endl;
  for(int i=0;i<n;i++){
    for(int j=0;j<n;j++){
      std::cout<< std::setw(5) <<S_[i][j].second << ",";
    }
    std::cout << std::endl;
  }
}

std::pair<double,double> DStarLite::CalculateKey(const Node& s){
  return std::make_pair(std::min(S_[s.x_][s.y_].first, S_[s.x_][s.y_].second
                                  +GetHeuristic(start_,s)+km_.first),
                        std::min(S_[s.x_][s.y_].first, S_[s.x_][s.y_].second));
}

std::vector<Node> DStarLite::GetPred(Node u){
  std::vector<Node> pred;
  for(auto it=motions.begin();it!=motions.end(); ++it){
    // Modify to prevent points already in the queue fro  being added?
    Node new_node = u + *it;
    if(new_node.x_ >= n || new_node.x_ < 0 ||
       new_node.y_ >= n || new_node.y_ < 0 ||
       grid[new_node.x_][new_node.y_]==1) continue;
     pred.push_back(new_node);
  }
  return pred;
}

std::vector<Node> DStarLite::GetSucc(Node u){
  std::vector<Node> succ;
  for(auto it=motions.begin();it!=motions.end(); ++it){
    Node new_node = u + *it;
    if(new_node.x_ < n && new_node.x_ >= 0 &&
       new_node.y_ < n && new_node.y_ >= 0 &&
       grid[new_node.x_][new_node.y_]!=1){
         succ.push_back(new_node);
    }
  }
  return succ;
}

void DStarLite::InsertionSort(){
   int nU = U_.size();
   int i, j;
   std::pair<Node,std::pair<double,double>> key;
   for (i = 1; i < nU; i++) {
       key = U_[i];
       j = i-1;
       while (j >= 0 && (U_[j].second.first > key.second.first
                          || (U_[j].second.first == key.second.first  && U_[j].second.second >= key.second.second))){
           U_[j+1] = U_[j];
           j--;
       }
       U_[j+1] = key;
   }
}

double DStarLite::C(Node s1, Node s2){
  if(s1.x_ < n && s1.x_ >= 0 && s1.y_ < n && s1.y_ >= 0 &&
     s2.x_ < n && s2.x_ >= 0 && s2.y_ < n && s2.y_ >= 0 &&
     grid[s1.x_][s1.y_] != 1 && grid[s2.x_][s2.y_] != 1){
    // Node diff = s2-s1;
    // for(auto it = motions.begin(); it!=motions.end(); ++it){
    //   if(diff == *it){
    //     return (*it).cost_;
    //   }
    // }
    return 1;
  }
  else{
    return n*n;
  }
}

void DStarLite::Init(){
  U_.clear();
  double n2 = n*n;
  large_num = std::make_pair(n2,n2);

  motions = GetMotion();

  km_=std::make_pair(0,0);

  std::vector<std::pair<double,double>> tmp(n);

  std::fill(tmp.begin(), tmp.end(), large_num);
  S_ = std::vector<std::vector<std::pair<double,double>>>(n) ;
  for (int i = 0; i < n; i++){
    S_[i] = tmp;
  }

  S_[goal_.x_][goal_.y_].second = 0;
  std::pair<Node, std::pair<double, double>> u_pair = std::make_pair(goal_, CalculateKey(goal_));
  U_.push_back(u_pair);
  InsertionSort();
}

void DStarLite::UpdateVertex(Node& u){
  if(u!=goal_){
    std::vector<Node> succ = GetSucc(u);
    double init_min = n*n;
    for(int i=0;i<succ.size();i++){
      double new_min = C(u,succ[i])+S_[succ[i].x_][succ[i].y_].first;
      if(new_min < init_min) init_min = new_min;
    }
    S_[u.x_][u.y_].second = init_min;
  }
  // can optimise following by using hash
  for(auto it = U_.begin(); it!=U_.end(); ++it){
    if((*it).first == u){
      U_.erase(it);
      break;
    }
  }
  if(S_[u.x_][u.y_].first != S_[u.x_][u.y_].second){
    std::pair<double,double> key = CalculateKey(u);
    U_.push_back(std::make_pair(u, key));
    InsertionSort();
  }
}

bool DStarLite::CompareKey(std::pair<double,double>& pair_in, Node& u){
  std::pair<double,double> node_key = CalculateKey(u);
  if(pair_in.first < node_key.first ||
    (pair_in.first == node_key.first && pair_in.second < node_key.second)){
    return true;
  }
  return false;
}

int DStarLite::ComputeShortestPath(){
  while((!U_.empty() && CompareKey(U_[0].second, start_)) || S_[start_.x_][start_.y_].first != S_[start_.x_][start_.y_].second){
    k_old_ = U_[0].second;
    Node u = U_[0].first;
    U_.erase(U_.begin());
    if(CompareKey(k_old_, u)){
      std::pair<Node, std::pair<double, double>> u_pair = std::make_pair(goal_, CalculateKey(goal_));
      U_.push_back(u_pair);
      InsertionSort();
    }
    else if (S_[u.x_][u.y_].first > S_[u.x_][u.y_].second){
      S_[u.x_][u.y_].first = S_[u.x_][u.y_].second;
      std::vector<Node> pred = GetPred(u);
      for(int i = 0;i<pred.size();i++){
        UpdateVertex(pred[i]);
      }
    }
    else{
      S_[u.x_][u.y_].first = n*n;
      std::vector<Node> pred = GetPred(u);
      for(int i = 0;i<pred.size();i++){
        UpdateVertex(pred[i]);
      }
      UpdateVertex(u);
    }
  }
  return 0;
}

std::vector<Node> DStarLite::d_star_lite(std::vector<std::vector<int>> &grid_in, Node start_in, Node goal_in){
  grid = grid_in;
  start_ = start_in;
  main_start_ = start_;
  goal_ = goal_in;
  last_ = start_;
  n = grid.size();
  Init();
  int ans = ComputeShortestPath();
  if(ans < 0 || S_[start_.x_][start_.y_].first==large_num.first){
    path_vector_.clear();
    Node no_path_node(-1,-1,-1,-1,-1);
    path_vector_.push_back(no_path_node);
    return path_vector_;
  }
  else{
    GeneratePathVector();
    return ReturnInvertedVector();
  }
}

std::vector<Node> DStarLite::SetObs(Node u){
  if(u==goal_ || u==start_){
    std::cout << "Cannot set current start or goal as obstacle" << std::endl;
    return path_vector_;
  }
  grid[u.x_][u.y_] = 1;
  std::cout << "Obstacle found at: " << std::endl;
  u.PrintStatus();
  DisplayGrid();
  return Replan(u);
}

std::vector<Node> DStarLite::Replan(Node u){
  if (grid[start_.x_][start_.y_]==1) grid[start_.x_][start_.y_]=0;
  path_vector_.clear();
  start_ = main_start_;
  std::vector<Node> succ;
  succ = GetSucc(start_);
  double init_min = n*n;
  double new_min = 0;
  Node new_start = Node(start_.x_, start_.y_);
  for(int i = 0;i<succ.size();i++){
    new_min = C(start_,succ[i])+S_[succ[i].x_][succ[i].y_].first;
    if(new_min < init_min){
      init_min = new_min;
      new_start = succ[i];
    }
  }
  start_ = new_start;
  km_.first = km_.first + GetHeuristic(last_,start_);
  last_ = start_;
  UpdateVertex(u);
  int ans = ComputeShortestPath();
  if(ans < 0 || S_[start_.x_][start_.y_].first==large_num.first){
    path_vector_.clear();
    Node no_path_node(-1,-1,-1,-1,-1);
    path_vector_.push_back(no_path_node);
    return path_vector_;
  }
  GeneratePathVector();
  return ReturnInvertedVector();
}

std::vector<Node> DStarLite::ReturnInvertedVector(){
  std::vector<Node> inverted_path_vector = path_vector_;
  // Inverting costs as dstar moves from goal to start.
  // Then inverting path vector for reordering.
  double start_cost = inverted_path_vector.back().cost_;
  for(auto it=inverted_path_vector.begin(); it!=inverted_path_vector.end(); ++it){
    (*it).cost_ = start_cost - (*it).cost_;
  }
  std::reverse(inverted_path_vector.begin(),inverted_path_vector.end());
  return inverted_path_vector;
}

void DStarLite::GeneratePathVector(){
  main_start_.cost_ = S_[main_start_.x_][main_start_.y_].second;
  path_vector_.push_back(main_start_);
  while(path_vector_[0]!=goal_){
    Node u = path_vector_[0];
    grid[u.x_][u.y_]=2;

    for(auto it=motions.begin();it!=motions.end(); ++it){
      Node new_node = u + *it;
      if(new_node.x_ >= n || new_node.x_ < 0 || new_node.y_ >= n || new_node.y_ < 0
         || grid[new_node.x_][new_node.y_]==1){
        continue;
      }
      if(new_node.x_ < n && new_node.x_ >= 0 && new_node.y_ < n && new_node.y_ >= 0){
        new_node.cost_= S_[new_node.x_][new_node.y_].second;
        if(new_node.cost_ > u.cost_){
           continue;
        }
        new_node.id_ = n*new_node.x_ + new_node.y_;
        new_node.pid_ = u.id_;
        path_vector_.push_back(new_node);
        VectorInsertionSort(path_vector_);
      }
    }
  }
  if(path_vector_[0]==goal_){
    grid[goal_.x_][goal_.y_]=2;
  }
}

std::vector<Node> DStarLite::UpdateStart(Node start_in){
  // Prevent teleportations
  if(path_vector_[0].cost_ == -1){
    std::cout << "Teleport disabled." << std::endl;
    return path_vector_;
  }
  // If no path found to goal from current start, not movement from start will
  // reach a point that can be reached from the goal. Teleportation is not
  // supported by D* Lite unless a second iter counter is added at the beginning
  // of the compute cost before the while loop.
  start_ = start_in;
  main_start_ = start_;
  km_.first = km_.first +GetHeuristic(last_, start_); //check of required
  last_ = start_;
  return path_vector_;
}

void DStarLite::DisplayGrid(){
  std::cout << "Grid: " << std::endl;
  std::cout << "1. Points not considered ---> 0" << std::endl;
  std::cout << "2. Obstacles             ---> 1" << std::endl;
  std::cout << "3. Points considered     ---> 2" << std::endl;
  std::cout << "4. Points in final path  ---> 3" << std::endl;
  std::cout << "5. Current point         ---> 4" << std::endl;
  for(int j=0;j<n;j++){
    std::cout << "---";
  }
  std::cout << std::endl;
  for(int i=0;i<n;i++){
    for(int j=0;j<n;j++){
      if(grid[i][j]==3) std::cout << GREEN << grid[i][j] << RESET << " , ";
      else if(grid[i][j]==1) std::cout << RED << grid[i][j] << RESET << " , ";
      else if(grid[i][j]==2) std::cout << BLUE << grid[i][j] << RESET << " , ";
      else if(grid[i][j]==4) std::cout << YELLOW << grid[i][j] << RESET << " , ";
      else std::cout << grid[i][j] << " , ";
    }
    std::cout << std::endl << std::endl;
  }
  for(int j=0;j<n;j++) std::cout <<  "---";
  std::cout << std::endl;
}

Node DStarLite::NextPoint(){
  int i = 0;
  for(i = 0; i < path_vector_.size(); i++){
    if(goal_ == path_vector_[i]){
      break;
    }
  }
  while(path_vector_[i].pid_!=start_.id_){
    for(int j = 0; j < path_vector_.size(); j++){
      if(path_vector_[i].pid_ == path_vector_[j].id_){
        i=j;
        break;
      }
    }
    if(path_vector_[i].pid_==start_.id_) break;
  }
  return path_vector_[i];
}

void DStarLite::RunDStarLite(bool disp_inc_in){
  disp_inc = disp_inc_in;
  if(path_vector_[0].cost_==-1){
    std::cout << "No path" << std::endl;
    return; // No path
  }
  std::random_device rd; // obtain a random number from hardware
  std::mt19937 eng(rd()); // seed the generator
  std::uniform_int_distribution<int> distr(0,n); // define the range
  Node current = path_vector_.back();
  while(current!=goal_){
    grid[start_.x_][start_.y_] = 3;
    UpdateStart(current);
    start_ = current;
    Node next_point = NextPoint();
    if(distr(eng) > n-2 && next_point!=goal_) SetObs(next_point);

    grid[current.x_][current.y_] = 4;
    if(disp_inc){
      usleep(disp_p);
      DisplayGrid();
    }
    if(path_vector_[0].cost_==-1){
      DisplayGrid(); // Shows path traversed and current point
      std::cout << "No path" << std::endl;
      return; // No path
    }
    start_ = current;
    current = NextPoint();
  }
  grid[start_.x_][start_.y_] = 3;
  grid[current.x_][current.y_] = 4;
  if(disp_inc) {
    usleep(disp_p);
    DisplayGrid();
  }
  grid[current.x_][current.y_] = 3;
  DisplayGrid();
  return;
}

#ifdef BUILD_INDIVIDUAL
/**
* @brief Script main function. Generates start and end nodes as well as grid, then creates the algorithm object and calls the main algorithm function.
* @return 0
*/
int main(){
  int n = 11;

  std::vector<std::vector<int>> grid(n);
  std::vector<int> tmp(n);
  for (int i = 0; i < n; i++){
    grid[i] = tmp;
  }
  MakeGrid(grid);
  std::random_device rd; // obtain a random number from hardware
  std::mt19937 eng(rd()); // seed the generator
  std::uniform_int_distribution<int> distr(0,n-1); // define the range

  Node start(distr(eng),distr(eng),0,0,0,0);
  Node goal(distr(eng),distr(eng),0,0,0,0);

  start.id_ = start.x_ * n + start.y_;
  start.pid_ = start.x_ * n + start.y_;
  goal.id_ = goal.x_ * n + goal.y_;
  start.h_cost_ = abs(start.x_ - goal.x_) + abs(start.y_ - goal.y_);
  //Make sure start and goal are not obstacles and their ids are correctly assigned.
  grid[start.x_][start.y_] = 0;
  grid[goal.x_][goal.y_] = 0;
  PrintGrid(grid);
  std::vector<Node> path_vector;
  DStarLite new_d_star_lite;

  path_vector = new_d_star_lite.d_star_lite(grid, start, goal);
  PrintPath(path_vector, start, goal, grid);

 new_d_star_lite.RunDStarLite();
  return 0;
}
#endif BUILD_INDIVIDUAL
