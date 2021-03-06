/*********************************************************************
 *
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2016,
 *  TU Dortmund - Institute of Control Theory and Systems Engineering.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of the institute nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 * Author: Christoph Rösmann
 *********************************************************************/


#ifndef ROBOT_FOOTPRINT_MODEL_H
#define ROBOT_FOOTPRINT_MODEL_H

#include <tseb_local_planner/pose_se2.h>
#include <tseb_local_planner/pose_se3.h>
#include <tseb_local_planner/obstacles.h>
#include <visualization_msgs/Marker.h>
#include <tseb_local_planner/misc.h>
#include <iostream>
namespace teb_local_planner
{

/**
 * @class BaseRobotFootprintModel
 * @brief Abstract class that defines the interface for robot footprint/contour models
 * 
 * The robot model class is currently used in optimization only, since
 * taking the navigation stack footprint into account might be
 * inefficient. The footprint is only used for checking feasibility.
 */
class BaseRobotFootprintModel
{
public:
  
  /**
    * @brief Default constructor of the abstract obstacle class
    */
  BaseRobotFootprintModel()
  {
  }
  
  /**
   * @brief Virtual destructor.
   */
  virtual ~BaseRobotFootprintModel()
  {
  }


  /**
    * @brief Calculate the distance between the robot and an obstacle
    * @param current_pose Current robot pose
    * @param obstacle Pointer to the obstacle
    * @return Euclidean distance to the robot
    */
  virtual double calculateDistance(const PoseSE2& current_pose, const Obstacle* obstacle) const = 0;

  /**
    * @brief Estimate the distance between the robot and the predicted location of an obstacle at time t
    * @param current_pose robot pose, from which the distance to the obstacle is estimated
    * @param obstacle Pointer to the dynamic obstacle (constant velocity model is assumed)
    * @param t time, for which the predicted distance to the obstacle is calculated
    * @return Euclidean distance to the robot
    */
  virtual double estimateSpatioTemporalDistance(const PoseSE3& current_pose, const Obstacle* obstacle) const = 0;

  /**
    * @brief Visualize the robot using a markers
    * 
    * Fill a marker message with all necessary information (type, pose, scale and color).
    * The header, namespace, id and marker lifetime will be overwritten.
    * @param current_pose Current robot pose
    * @param[out] markers container of marker messages describing the robot shape
    * @param color Color of the footprint
    */
  virtual void visualizeRobot(const PoseSE2& current_pose, std::vector<visualization_msgs::Marker>& markers, const std_msgs::ColorRGBA& color) const {}
  
  
  /**
   * @brief Compute the inscribed radius of the footprint model
   * @return inscribed radius
   */
  virtual double getInscribedRadius() = 0;
  virtual double getCircumscribedRadius() const= 0;
  virtual Eigen::Vector2d getClosestPoint(const PoseSE3& current_pose,const Eigen::Vector2d& point) const= 0 ;
	

public:	
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};


//! Abbrev. for shared obstacle pointers
typedef boost::shared_ptr<BaseRobotFootprintModel> RobotFootprintModelPtr;
//! Abbrev. for shared obstacle const pointers
typedef boost::shared_ptr<const BaseRobotFootprintModel> RobotFootprintModelConstPtr;



/**
 * @class PointRobotShape
 * @brief Class that defines a point-robot
 * 
 * Instead of using a CircularRobotFootprint this class might
 * be utitilzed and the robot radius can be added to the mininum distance 
 * parameter. This avoids a subtraction of zero each time a distance is calculated.
 */
class PointRobotFootprint : public BaseRobotFootprintModel
{
public:
  
  /**
    * @brief Default constructor of the abstract obstacle class
    */
  PointRobotFootprint() {}
  
  /**
   * @brief Virtual destructor.
   */
  virtual ~PointRobotFootprint() {}

  /**
    * @brief Calculate the distance between the robot and an obstacle
    * @param current_pose Current robot pose
    * @param obstacle Pointer to the obstacle
    * @return Euclidean distance to the robot
    */
  virtual double calculateDistance(const PoseSE2& current_pose, const Obstacle* obstacle) const
  {
    return obstacle->getMinimumDistance(current_pose.position());
  }
  
  /**
    * @brief Estimate the distance between the robot and the predicted location of an obstacle at time t
    * @param current_pose robot pose, from which the distance to the obstacle is estimated
    * @param obstacle Pointer to the dynamic obstacle (constant velocity model is assumed)
    * @param t time, for which the predicted distance to the obstacle is calculated
    * @return Euclidean distance to the robot
    */
  virtual double estimateSpatioTemporalDistance(const PoseSE3& current_pose, const Obstacle* obstacle) const
  {
    return obstacle->getMinimumSpatioTemporalDistance(current_pose);
  }

  /**
   * @brief Compute the inscribed radius of the footprint model
   * @return inscribed radius
   */
  virtual double getInscribedRadius() {return 0.0;}
  virtual double getCircumscribedRadius() const{return 0.0;};
  virtual Eigen::Vector2d getClosestPoint(const PoseSE3& current_pose,const Eigen::Vector2d& point) const
  {
    return current_pose.position();
  }

};


/**
 * @class CircularRobotFootprint
 * @brief Class that defines the a robot of circular shape
 */
class CircularRobotFootprint : public BaseRobotFootprintModel
{
public:
  
  /**
    * @brief Default constructor of the abstract obstacle class
    * @param radius radius of the robot
    */
  CircularRobotFootprint(double radius) : radius_(radius) { }
  
  /**
   * @brief Virtual destructor.
   */
  virtual ~CircularRobotFootprint() { }

  /**
    * @brief Set radius of the circular robot
    * @param radius radius of the robot
    */
  void setRadius(double radius) {radius_ = radius;}
  
  /**
    * @brief Calculate the distance between the robot and an obstacle
    * @param current_pose Current robot pose
    * @param obstacle Pointer to the obstacle
    * @return Euclidean distance to the robot
    */
  virtual double calculateDistance(const PoseSE2& current_pose, const Obstacle* obstacle) const
  {
    return obstacle->getMinimumDistance(current_pose.position()) - radius_;
  }

  /**
    * @brief Estimate the distance between the robot and the predicted location of an obstacle at time t
    * @param current_pose robot pose, from which the distance to the obstacle is estimated
    * @param obstacle Pointer to the dynamic obstacle (constant velocity model is assumed)
    * @param t time, for which the predicted distance to the obstacle is calculated
    * @return Euclidean distance to the robot
    */
  virtual double estimateSpatioTemporalDistance(const PoseSE3& current_pose, const Obstacle* obstacle) const
  {
    return obstacle->getMinimumSpatioTemporalDistance(current_pose,radius_);
  }

  /**
    * @brief Visualize the robot using a markers
    * 
    * Fill a marker message with all necessary information (type, pose, scale and color).
    * The header, namespace, id and marker lifetime will be overwritten.
    * @param current_pose Current robot pose
    * @param[out] markers container of marker messages describing the robot shape
    * @param color Color of the footprint
    */
  virtual void visualizeRobot(const PoseSE2& current_pose, std::vector<visualization_msgs::Marker>& markers, const std_msgs::ColorRGBA& color) const
  {
    markers.resize(1);
    visualization_msgs::Marker& marker = markers.back();
    marker.type = visualization_msgs::Marker::CYLINDER;
    current_pose.toPoseMsg(marker.pose);
    marker.scale.x = marker.scale.y = 2*radius_; // scale = diameter
    marker.scale.z = 0.05;
    marker.color = color;
  }
  
  /**
   * @brief Compute the inscribed radius of the footprint model
   * @return inscribed radius
   */
  virtual double getInscribedRadius() {return radius_;}
  virtual double getCircumscribedRadius() const{return radius_;}
  virtual Eigen::Vector2d getClosestPoint(const PoseSE3& current_pose,const Eigen::Vector2d& point) const
  {
      Eigen::Vector2d direction = point-current_pose.position();
      direction.normalize();
    return current_pose.position()+radius_*direction;
  }
private:
    
  double radius_;
};


/**
 * @class TwoCirclesRobotFootprint
 * @brief Class that approximates the robot with two shifted circles
 */
class TwoCirclesRobotFootprint : public BaseRobotFootprintModel
{
public:
  
  /**
    * @brief Default constructor of the abstract obstacle class
    * @param front_offset shift the center of the front circle along the robot orientation starting from the center at the rear axis (in meters)
    * @param front_radius radius of the front circle
    * @param rear_offset shift the center of the rear circle along the opposite robot orientation starting from the center at the rear axis (in meters)
    * @param rear_radius radius of the front circle
    */
  TwoCirclesRobotFootprint(double front_offset, double front_radius, double rear_offset, double rear_radius) 
    : front_offset_(front_offset), front_radius_(front_radius), rear_offset_(rear_offset), rear_radius_(rear_radius) { }
  
  /**
   * @brief Virtual destructor.
   */
  virtual ~TwoCirclesRobotFootprint() { }

  /**
   * @brief Set parameters of the contour/footprint
   * @param front_offset shift the center of the front circle along the robot orientation starting from the center at the rear axis (in meters)
   * @param front_radius radius of the front circle
   * @param rear_offset shift the center of the rear circle along the opposite robot orientation starting from the center at the rear axis (in meters)
   * @param rear_radius radius of the front circle
   */
  void setParameters(double front_offset, double front_radius, double rear_offset, double rear_radius) 
  {front_offset_=front_offset; front_radius_=front_radius; rear_offset_=rear_offset; rear_radius_=rear_radius;}
  
  /**
    * @brief Calculate the distance between the robot and an obstacle
    * @param current_pose Current robot pose
    * @param obstacle Pointer to the obstacle
    * @return Euclidean distance to the robot
    */
  virtual double calculateDistance(const PoseSE2& current_pose, const Obstacle* obstacle) const
  {
    Eigen::Vector2d dir = current_pose.orientationUnitVec();
    double dist_front = obstacle->getMinimumDistance(current_pose.position() + front_offset_*dir) - front_radius_;
    double dist_rear = obstacle->getMinimumDistance(current_pose.position() - rear_offset_*dir) - rear_radius_;
    return std::min(dist_front, dist_rear);
  }

  /**
    * @brief Estimate the distance between the robot and the predicted location of an obstacle at time t
    * @param current_pose robot pose, from which the distance to the obstacle is estimated
    * @param obstacle Pointer to the dynamic obstacle (constant velocity model is assumed)
    * @param t time, for which the predicted distance to the obstacle is calculated
    * @return Euclidean distance to the robot
    */
  virtual double estimateSpatioTemporalDistance(const PoseSE3& current_pose, const Obstacle* obstacle) const
  {
    Eigen::Vector2d dir = current_pose.orientationUnitVec();
    PoseSE3 front_pose = current_pose;
    PoseSE3 rear_pose = current_pose;
    front_pose.position() = front_pose.position() + front_offset_*dir;
    rear_pose.position() = rear_pose.position() - rear_offset_*dir;
    double dist_front = obstacle->getMinimumSpatioTemporalDistance(front_pose,front_radius_);
    double dist_rear = obstacle->getMinimumSpatioTemporalDistance(rear_pose,rear_radius_);
    return std::min(dist_front, dist_rear);
  }

  /**
    * @brief Visualize the robot using a markers
    * 
    * Fill a marker message with all necessary information (type, pose, scale and color).
    * The header, namespace, id and marker lifetime will be overwritten.
    * @param current_pose Current robot pose
    * @param[out] markers container of marker messages describing the robot shape
    * @param color Color of the footprint
    */
  virtual void visualizeRobot(const PoseSE2& current_pose, std::vector<visualization_msgs::Marker>& markers, const std_msgs::ColorRGBA& color) const
  {    
    Eigen::Vector2d dir = current_pose.orientationUnitVec();
    if (front_radius_>0)
    {
      markers.push_back(visualization_msgs::Marker());
      visualization_msgs::Marker& marker1 = markers.back();
      marker1.type = visualization_msgs::Marker::CYLINDER;
      current_pose.toPoseMsg(marker1.pose);
      marker1.pose.position.x += front_offset_*dir.x();
      marker1.pose.position.y += front_offset_*dir.y();
      marker1.scale.x = marker1.scale.y = 2*front_radius_; // scale = diameter
//       marker1.scale.z = 0.05;
      marker1.color = color;

    }
    if (rear_radius_>0)
    {
      markers.push_back(visualization_msgs::Marker());
      visualization_msgs::Marker& marker2 = markers.back();
      marker2.type = visualization_msgs::Marker::CYLINDER;
      current_pose.toPoseMsg(marker2.pose);
      marker2.pose.position.x -= rear_offset_*dir.x();
      marker2.pose.position.y -= rear_offset_*dir.y();
      marker2.scale.x = marker2.scale.y = 2*rear_radius_; // scale = diameter
//       marker2.scale.z = 0.05;
      marker2.color = color;
    }
  }
  
  /**
   * @brief Compute the inscribed radius of the footprint model
   * @return inscribed radius
   */
  virtual double getInscribedRadius() 
  {
      double min_longitudinal = std::min(rear_offset_ + rear_radius_, front_offset_ + front_radius_);
      double min_lateral = std::min(rear_radius_, front_radius_);
      return std::min(min_longitudinal, min_lateral);
  }
  virtual double getCircumscribedRadius() const
  {
      return 0.5*(rear_offset_ + rear_radius_+front_offset_ + front_radius_);
  }
  virtual Eigen::Vector2d getClosestPoint(const PoseSE3& current_pose,const Eigen::Vector2d& point) const
  {
      Eigen::Vector2d dir = current_pose.orientationUnitVec();
      PoseSE3 front_pose = current_pose;
      PoseSE3 rear_pose = current_pose;
      front_pose.position() = front_pose.position() + front_offset_*dir;
      rear_pose.position() = rear_pose.position() - rear_offset_*dir;
      Eigen::Vector2d direction_front = point-front_pose.position();
      double dist_front = direction_front.norm();
      Eigen::Vector2d direction_rear = point-rear_pose.position();
      double dist_rear = direction_rear.norm();
      if (dist_rear<dist_front)
      {
          direction_front.normalize();
          return current_pose.position()+front_radius_*direction_front;
      }else
      {
          direction_rear.normalize();
          return current_pose.position()+rear_radius_*direction_rear;
      }
  }

private:
    
  double front_offset_;
  double front_radius_;
  double rear_offset_;
  double rear_radius_;
  
};



/**
 * @class LineRobotFootprint
 * @brief Class that approximates the robot with line segment (zero-width)
 */
class LineRobotFootprint : public BaseRobotFootprintModel
{
public:
  
  /**
    * @brief Default constructor of the abstract obstacle class
    * @param line_start start coordinates (only x and y) of the line (w.r.t. robot center at (0,0))
    * @param line_end end coordinates (only x and y) of the line (w.r.t. robot center at (0,0))
    */
  LineRobotFootprint(const geometry_msgs::Point& line_start, const geometry_msgs::Point& line_end)
  {
    setLine(line_start, line_end);
  }
  
  /**
  * @brief Default constructor of the abstract obstacle class (Eigen Version)
  * @param line_start start coordinates (only x and y) of the line (w.r.t. robot center at (0,0))
  * @param line_end end coordinates (only x and y) of the line (w.r.t. robot center at (0,0))
  */
  LineRobotFootprint(const Eigen::Vector2d& line_start, const Eigen::Vector2d& line_end)
  {
    setLine(line_start, line_end);
  }
  
  /**
   * @brief Virtual destructor.
   */
  virtual ~LineRobotFootprint() { }

  /**
   * @brief Set vertices of the contour/footprint
   * @param vertices footprint vertices (only x and y) around the robot center (0,0) (do not repeat the first and last vertex at the end)
   */
  void setLine(const geometry_msgs::Point& line_start, const geometry_msgs::Point& line_end)
  {
    line_start_.x() = line_start.x; 
    line_start_.y() = line_start.y; 
    line_end_.x() = line_end.x;
    line_end_.y() = line_end.y;
  }
  
  /**
   * @brief Set vertices of the contour/footprint (Eigen version)
   * @param vertices footprint vertices (only x and y) around the robot center (0,0) (do not repeat the first and last vertex at the end)
   */
  void setLine(const Eigen::Vector2d& line_start, const Eigen::Vector2d& line_end)
  {
    line_start_ = line_start; 
    line_end_ = line_end;
  }
  
  /**
    * @brief Calculate the distance between the robot and an obstacle
    * @param current_pose Current robot pose
    * @param obstacle Pointer to the obstacle
    * @return Euclidean distance to the robot
    */
  virtual double calculateDistance(const PoseSE2& current_pose, const Obstacle* obstacle) const
  {
    Eigen::Vector2d line_start_world;
    Eigen::Vector2d line_end_world;
    transformToWorld(current_pose, line_start_world, line_end_world);
    return obstacle->getMinimumDistance(line_start_world, line_end_world);
  }

  /**
    * @brief Estimate the distance between the robot and the predicted location of an obstacle at time t
    * @param current_pose robot pose, from which the distance to the obstacle is estimated
    * @param obstacle Pointer to the dynamic obstacle (constant velocity model is assumed)
    * @param t time, for which the predicted distance to the obstacle is calculated
    * @return Euclidean distance to the robot
    */
  virtual double estimateSpatioTemporalDistance(const PoseSE3& current_pose, const Obstacle* obstacle) const
  {
    Eigen::Vector2d line_start_world;
    Eigen::Vector2d line_end_world;
    transformToWorld(current_pose.toPoseSE2(), line_start_world, line_end_world);
    PoseSE3 line_start(line_start_world,current_pose.t(),current_pose.theta());
    PoseSE3 line_end(line_end_world,current_pose.t(),current_pose.theta());
    return obstacle->getMinimumSpatioTemporalDistance(line_start, line_end);
  }

  /**
    * @brief Visualize the robot using a markers
    * 
    * Fill a marker message with all necessary information (type, pose, scale and color).
    * The header, namespace, id and marker lifetime will be overwritten.
    * @param current_pose Current robot pose
    * @param[out] markers container of marker messages describing the robot shape
    * @param color Color of the footprint
    */
  virtual void visualizeRobot(const PoseSE2& current_pose, std::vector<visualization_msgs::Marker>& markers, const std_msgs::ColorRGBA& color) const
  {   
    markers.push_back(visualization_msgs::Marker());
    visualization_msgs::Marker& marker = markers.back();
    marker.type = visualization_msgs::Marker::LINE_STRIP;
    current_pose.toPoseMsg(marker.pose); // all points are transformed into the robot frame!
    
    // line
    geometry_msgs::Point line_start_world;
    line_start_world.x = line_start_.x();
    line_start_world.y = line_start_.y();
    line_start_world.z = 0;
    marker.points.push_back(line_start_world);
    
    geometry_msgs::Point line_end_world;
    line_end_world.x = line_end_.x();
    line_end_world.y = line_end_.y();
    line_end_world.z = 0;
    marker.points.push_back(line_end_world);

    marker.scale.x = 0.05; 
    marker.color = color;
  }
  
  /**
   * @brief Compute the inscribed radius of the footprint model
   * @return inscribed radius
   */
  virtual double getInscribedRadius() 
  {
      return 0.0; // lateral distance = 0.0
  }
  virtual double getCircumscribedRadius() const
  {
      return (line_end_-line_start_).norm()*0.5; // lateral distance = 0.0
  }
  virtual Eigen::Vector2d getClosestPoint(const PoseSE3& current_pose,const Eigen::Vector2d& point) const
  {
      Eigen::Vector2d cross_point;
      computeDistPointToLine(point,line_start_,line_end_,&cross_point);
      return cross_point;
  }
private:
    
  /**
    * @brief Transforms a line to the world frame manually
    * @param current_pose Current robot pose
    * @param[out] line_start line_start_ in the world frame
    * @param[out] line_end line_end_ in the world frame
    */
  void transformToWorld(const PoseSE2& current_pose, Eigen::Vector2d& line_start_world, Eigen::Vector2d& line_end_world) const
  {
    double cos_th = std::cos(current_pose.theta());
    double sin_th = std::sin(current_pose.theta());
    line_start_world.x() = current_pose.x() + cos_th * line_start_.x() - sin_th * line_start_.y();
    line_start_world.y() = current_pose.y() + sin_th * line_start_.x() + cos_th * line_start_.y();
    line_end_world.x() = current_pose.x() + cos_th * line_end_.x() - sin_th * line_end_.y();
    line_end_world.y() = current_pose.y() + sin_th * line_end_.x() + cos_th * line_end_.y();
  }

  Eigen::Vector2d line_start_;
  Eigen::Vector2d line_end_;
  
public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  
};



/**
 * @class PolygonRobotFootprint
 * @brief Class that approximates the robot with a closed polygon
 */
class PolygonRobotFootprint : public BaseRobotFootprintModel
{
public:
  
  
  
  /**
    * @brief Default constructor of the abstract obstacle class
    * @param vertices footprint vertices (only x and y) around the robot center (0,0) (do not repeat the first and last vertex at the end)
    */
  PolygonRobotFootprint(const Point2dContainer& vertices) : vertices_(vertices) { }
  
  /**
   * @brief Virtual destructor.
   */
  virtual ~PolygonRobotFootprint() { }

  /**
   * @brief Set vertices of the contour/footprint
   * @param vertices footprint vertices (only x and y) around the robot center (0,0) (do not repeat the first and last vertex at the end)
   */
  void setVertices(const Point2dContainer& vertices) {vertices_ = vertices;}
  
  /**
    * @brief Calculate the distance between the robot and an obstacle
    * @param current_pose Current robot pose
    * @param obstacle Pointer to the obstacle
    * @return Euclidean distance to the robot
    */
  virtual double calculateDistance(const PoseSE2& current_pose, const Obstacle* obstacle) const
  {
    Point2dContainer polygon_world(vertices_.size());
    transformToWorld(current_pose, polygon_world);
    return obstacle->getMinimumDistance(polygon_world);
  }

  /**
    * @brief Estimate the distance between the robot and the predicted location of an obstacle at time t
    * @param current_pose robot pose, from which the distance to the obstacle is estimated
    * @param obstacle Pointer to the dynamic obstacle (constant velocity model is assumed)
    * @param t time, for which the predicted distance to the obstacle is calculated
    * @return Euclidean distance to the robot
    */
  virtual double estimateSpatioTemporalDistance(const PoseSE3& current_pose, const Obstacle* obstacle) const
  {
    Point2dContainer polygon_world(vertices_.size());
    transformToWorld(current_pose.toPoseSE2(), polygon_world);
    Point3dContainer polygon_3d(vertices_.size());
    for (unsigned int i=0;i<polygon_world.size();++i)
    {
        polygon_3d[i][0] = polygon_world[i][0];
        polygon_3d[i][1] = polygon_world[i][1];
        polygon_3d[i][2] = current_pose.t();
    }
    return obstacle->getMinimumSpatioTemporalDistance(polygon_3d);
  }

  /**
    * @brief Visualize the robot using a markers
    * 
    * Fill a marker message with all necessary information (type, pose, scale and color).
    * The header, namespace, id and marker lifetime will be overwritten.
    * @param current_pose Current robot pose
    * @param[out] markers container of marker messages describing the robot shape
    * @param color Color of the footprint
    */
  virtual void visualizeRobot(const PoseSE2& current_pose, std::vector<visualization_msgs::Marker>& markers, const std_msgs::ColorRGBA& color) const
  {
    if (vertices_.empty())
      return;

    markers.push_back(visualization_msgs::Marker());
    visualization_msgs::Marker& marker = markers.back();
    marker.type = visualization_msgs::Marker::LINE_STRIP;
    current_pose.toPoseMsg(marker.pose); // all points are transformed into the robot frame!
    
    for (std::size_t i = 0; i < vertices_.size(); ++i)
    {
      geometry_msgs::Point point;
      point.x = vertices_[i].x();
      point.y = vertices_[i].y();
      point.z = 0;
      marker.points.push_back(point);
    }
    // add first point again in order to close the polygon
    geometry_msgs::Point point;
    point.x = vertices_.front().x();
    point.y = vertices_.front().y();
    point.z = 0;
    marker.points.push_back(point);

    marker.scale.x = 0.025; 
    marker.color = color;

  }
  
  /**
   * @brief Compute the inscribed radius of the footprint model
   * @return inscribed radius
   */
  virtual double getCircumscribedRadius() const
  {
     double max_dist = std::numeric_limits<double>::min();
     Eigen::Vector2d center(0.0, 0.0);
      
     if (vertices_.size() <= 2)
        return 0.0;

     for (int i = 0; i < (int)vertices_.size(); ++i)
     {
        // compute distance from the robot center point to the first vertex
        double vertex_dist = vertices_[i].norm();
        //double edge_dist = distance_point_to_segment_2d(center, vertices_[i], vertices_[i+1]);
        max_dist = std::max(max_dist, vertex_dist);
     }
 
     return max_dist;
  }
  virtual double getInscribedRadius()
  {
     double min_dist = std::numeric_limits<double>::max();
     Eigen::Vector2d center(0.0, 0.0);

     if (vertices_.size() <= 2)
        return 0.0;

     for (int i = 0; i < (int)vertices_.size() - 1; ++i)
     {
        // compute distance from the robot center point to the first vertex
        double vertex_dist = vertices_[i].norm();
        double edge_dist = distance_point_to_segment_2d(center, vertices_[i], vertices_[i+1]);
        min_dist = std::min(min_dist, std::min(vertex_dist, edge_dist));
     }

     // we also need to check the last vertex and the first vertex
     double vertex_dist = vertices_.back().norm();
     double edge_dist = distance_point_to_segment_2d(center, vertices_.back(), vertices_.front());
     return std::min(min_dist, std::min(vertex_dist, edge_dist));
  }
  virtual Eigen::Vector2d getClosestPoint(const PoseSE3& current_pose,const Eigen::Vector2d& point) const
  {
    Point2dContainer polygon_world(vertices_.size());
    transformToWorld(current_pose.toPoseSE2(), polygon_world);
      Eigen::Vector2d cross_point(0,0);
      std::cout<<"pose :"<<current_pose.x()<<" "<<current_pose.y()<<std::endl;
      std::cout<<"point:"<<point[0]<<" "<<point[1]<<std::endl;
      if (inPolygon(point,polygon_world))
        return point;
      std::cout<<"polygon:"<<std::endl;
      for (int i = 0; i < (int)polygon_world.size(); ++i)
      {
          std::cout<<polygon_world[i]<<std::endl;
        int j=(i+1)%(int)polygon_world.size();
         // compute distance from the robot center point to the first vertex
         if (check_line_segments_intersection_2d(point,current_pose.position(),polygon_world[i],polygon_world[j],&cross_point))
             return cross_point;

      }
      //if (check_line_segments_intersection_2d(point,current_pose.position(),vertices_[vertices_.size() - 1],vertices_[0],&cross_point))
          //return cross_point;

      ROS_WARN("WARN:not find a cross point");
      return cross_point;
  }
private:
    
  /**
    * @brief Transforms a polygon to the world frame manually
    * @param current_pose Current robot pose
    * @param[out] polygon_world polygon in the world frame
    */
  void transformToWorld(const PoseSE2& current_pose, Point2dContainer& polygon_world) const
  {
    double cos_th = std::cos(current_pose.theta());
    double sin_th = std::sin(current_pose.theta());
    for (std::size_t i=0; i<vertices_.size(); ++i)
    {
      polygon_world[i].x() = current_pose.x() + cos_th * vertices_[i].x() - sin_th * vertices_[i].y();
      polygon_world[i].y() = current_pose.y() + sin_th * vertices_[i].x() + cos_th * vertices_[i].y();
    }
  }

  Point2dContainer vertices_;
  
};





} // namespace teb_local_planner

#endif /* ROBOT_FOOTPRINT_MODEL_H */
