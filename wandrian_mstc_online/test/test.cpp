/*
 * test.cpp
 *
 *  Created on: Jul 31, 2015
 *      Author: sontd
 */

#include <GL/glut.h>
#include <limits>
#include <set>
#include <ctime>
#include <cstdlib>
#include <stdlib.h>
#include <boost/next_prior.hpp>
#include <boost/bind.hpp>
#include <sstream>
#include <fstream>
#include "../include/common/environment.hpp"
#include "../include/plans/spiral_stc/full_spiral_stc.hpp"

#define R_SIZE 0.5 // Robot size
#define E_SIZE 4.0 // Default environment size
#define WORLD_INSERT_OBSTACLE "<!-- INSERT: Bound and Obstacles here -->" // Flag at original world file to insert bound and obstacles into

using namespace wandrian::plans::spiral_stc;

double e_size = 0;

EnvironmentPtr environment;
PointPtr starting_point_1;
PointPtr starting_point_2;
std::list<PointPtr> tmp_path_1;
std::list<PointPtr> tmp_path_2;

/**
 * Linked libraries to compile: -lglut -lGL (g++)
 */

void draw(std::list<PointPtr> points, int type) {
  glBegin(type);
  for (std::list<PointPtr>::iterator p = points.begin(); p != points.end();
      p++) {
    glVertex2d((*p)->x, (*p)->y);
  }
  glVertex2d((*points.begin())->x, (*points.begin())->y);
  glEnd();
}

void display() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-6, 6, -6, 6, -1, 1);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glScalef(e_size <= 20 ? 0.5 : 10.0 / e_size,
      e_size <= 20 ? 0.5 : 10.0 / e_size, 0);

  // Center point
  glPointSize(4);
  glColor3ub(255, 255, 0);
  glBegin(GL_POINTS);
  glVertex2i(0, 0);
  glEnd();

  // Coordinate
  glPointSize(1);
  glColor3ub(255, 255, 255);
  glBegin(GL_POINTS);
  for (int i = -e_size; i <= e_size; i++) {
    for (int j = -e_size; j <= e_size; j++) {
      if ((i != 0 || j != 0) && i % 2 == 0 && j % 2 == 0)
        glVertex2i((double) i / 2, (double) j / 2);
    }
  }
  glEnd();

  // Environment
  glColor3ub(255, 0, 0);
  draw(environment->space->get_bound(), GL_LINE_STRIP);
  for (std::list<PolygonPtr>::iterator obstacle =
      environment->obstacles.begin(); obstacle != environment->obstacles.end();
      obstacle++) {
    draw((*obstacle)->get_bound(), GL_POLYGON);
  }

  // Starting point 1
  glPointSize(4);
  glColor3ub(0, 255, 0);
  glBegin(GL_POINTS);
  glVertex2d(starting_point_1->x, starting_point_1->y);
  glEnd();

  // Starting point 2
  glPointSize(4);
  glColor3ub(0, 255, 0);
  glBegin(GL_POINTS);
  glVertex2d(starting_point_2->x, starting_point_2->y);
  glEnd();
  
  glRasterPos2i(0, e_size <= 20 ? -11 : -e_size / 2 - 1);
  std::stringstream ss_1;
  ss_1 << starting_point_1->x << ", " << starting_point_1->y;
  for (int i = 0; i < ss_1.str().length(); i++)
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, ss_1.str()[i]);

  glRasterPos2i(0, e_size <= 20 ? -11 : -e_size / 2 - 1);
  std::stringstream ss_2;
  ss_2 << starting_point_2->x << ", " << starting_point_2->y;
  for (int i = 0; i < ss_2.str().length(); i++)
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, ss_1.str()[i]);

  // Spiral STC covering path
  glColor3ub(0, 255, 0);
  draw(tmp_path_1, GL_LINE_STRIP);
  draw(tmp_path_2, GL_LINE_STRIP);

  glutSwapBuffers();
}

int run(int argc, char **argv) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
  glutInitWindowSize(600, 600);
  glutCreateWindow("Environment");
  glutDisplayFunc(display);
  glutMainLoop();
  return 0;
}

bool test_go_to(PointPtr position, bool flexibly) {
  tmp_path.insert(tmp_path.end(), position);
  return true;
}

bool test_see_obstacle(VectorPtr orientation, double distance) {
  // Simulator check obstacle
  PointPtr last_position = *(--tmp_path.end());
  PointPtr new_position = last_position + orientation * distance;
  if (environment) {
    CellPtr space = boost::static_pointer_cast<Cell>(environment->space);
    if (new_position->x >= space->get_center()->x + space->get_size() / 2
        || new_position->x <= space->get_center()->x - space->get_size() / 2
        || new_position->y >= space->get_center()->y + space->get_size() / 2
        || new_position->y <= space->get_center()->y - space->get_size() / 2) {
      return true;
    }
    for (std::list<PolygonPtr>::iterator o = environment->obstacles.begin();
        o != environment->obstacles.end(); o++) {
      CellPtr obstacle = boost::static_pointer_cast<Cell>(*o);
      if (new_position->x
          >= obstacle->get_center()->x - obstacle->get_size() / 2
          && new_position->x
              <= obstacle->get_center()->x + obstacle->get_size() / 2
          && new_position->y
              >= obstacle->get_center()->y - obstacle->get_size() / 2
          && new_position->y
              <= obstacle->get_center()->y + obstacle->get_size() / 2) {
        return true;
      }
    }
  }
  return false;
}

int main(int argc, char **argv) {
  if (argc >= 2) {
    std::istringstream iss(argv[1]);
    if (!(iss >> e_size) || !((int) e_size % 2 == 0)) {
      e_size = E_SIZE;
    }
  } else {
    e_size = E_SIZE;
  }
  CellPtr space = CellPtr(new Cell(PointPtr(new Point(0, 0)), e_size));
  std::list<PolygonPtr> obstacles;

  std::srand(std::time(0));
  starting_point = PointPtr(
      new Point(
          (std::rand() % (int) (e_size / R_SIZE / 2.0)
              - (int) (e_size / R_SIZE / 4.0)) + R_SIZE + R_SIZE / 2.0,
          (std::rand() % (int) (e_size / R_SIZE / 2.0)
              - (int) (e_size / R_SIZE / 4.0)) + R_SIZE - R_SIZE / 2.0));

  double o_size;
  int number_of_obstacles;
  double r = 0.2;
  if (argc >= 3) {
    std::istringstream iss(argv[2]);
    if (!(iss >> o_size)) {
      o_size = 2.0 * R_SIZE;
      number_of_obstacles = 0;
    } else {
      double n = 0.75 * r * (e_size * e_size) / (o_size * o_size);
      number_of_obstacles = n
          + ((int) (0.25 * n) != 0 ? std::rand() % (int) (0.25 * n) : 0);
    }
  } else {
    o_size = 2.0 * R_SIZE;
    double n = 0.75 * r * (e_size * e_size) / (o_size * o_size);
    number_of_obstacles = n
        + ((int) (0.25 * n) != 0 ? std::rand() % (int) (0.25 * n) : 0);
  }

  for (int i = 0; i <= number_of_obstacles; i++) {
    PointPtr center = PointPtr(
        new Point(
            (std::rand() % (int) (e_size / R_SIZE / (o_size / R_SIZE))
                - (int) (e_size / R_SIZE / (o_size / R_SIZE) / 2.0) + R_SIZE)
                * o_size,
            (std::rand() % (int) (e_size / R_SIZE / (o_size / R_SIZE))
                - (int) (e_size / R_SIZE / (o_size / R_SIZE) / 2.0) + R_SIZE)
                * o_size));
    bool valid = true;
    double EPS = std::numeric_limits<double>::epsilon();
    for (std::list<PolygonPtr>::iterator p = obstacles.begin();
        p != obstacles.end(); p++)
      if ((boost::static_pointer_cast<Cell>(*p))->get_center() == center
          || (std::abs(center->x - (starting_point->x - R_SIZE / 2)) < EPS
              && std::abs(center->y - (starting_point->y + R_SIZE / 2)) < EPS)) {
        valid = false;
        break;
      };
    if (valid) {
      obstacles.insert(obstacles.end(), CellPtr(new Cell(center, o_size)));
    }
  }

  std::ifstream world_in("../../worlds/empty.world");
  std::ofstream world_out("../../worlds/tmp.world");
  std::string line;
  while (std::getline(world_in, line, '\n')) {
    world_out << line << '\n';
    if (line.find(WORLD_INSERT_OBSTACLE) != std::string::npos) {
      int n;
      n = 1;
      // Upper bound
      for (double i = -e_size / 2 + R_SIZE / 2; i <= e_size / 2 - R_SIZE / 2;
          i +=
          R_SIZE) {
        world_out << "    <model name='cinder_block_bound_" << n << "'>\n";
        world_out << "      <include>\n";
        world_out << "        <uri>model://cinder_block</uri>\n";
        world_out << "      </include>\n";
        world_out << "      <pose>" << i << " " << (e_size / 2 + R_SIZE / 4)
            << " 0 0 0 0</pose>\n";
        world_out << "      <static>1</static>\n";
        world_out << "    </model>\n";
        n++;
      }

      // Right bound
      for (double i = -e_size / 2 + R_SIZE / 2; i <= e_size / 2 - R_SIZE / 2;
          i +=
          R_SIZE) {
        world_out << "    <model name='cinder_block_bound_" << n << "'>\n";
        world_out << "      <include>\n";
        world_out << "        <uri>model://cinder_block</uri>\n";
        world_out << "      </include>\n";
        world_out << "      <pose>" << (e_size / 2 + R_SIZE / 4) << " " << -i
            << " 0 0 0 " << M_PI_2 << "</pose>\n";
        world_out << "      <static>1</static>\n";
        world_out << "    </model>\n";
        n++;
      }

      // Lower bound
      for (double i = -e_size / 2 + R_SIZE / 2; i <= e_size / 2 - R_SIZE / 2;
          i +=
          R_SIZE) {
        world_out << "    <model name='cinder_block_bound_" << n << "'>\n";
        world_out << "      <include>\n";
        world_out << "        <uri>model://cinder_block</uri>\n";
        world_out << "      </include>\n";
        world_out << "      <pose>" << -i << " " << -(e_size / 2 + R_SIZE / 4)
            << " 0 0 0 0</pose>\n";
        world_out << "      <static>1</static>\n";
        world_out << "    </model>\n";
        n++;
      }

      // Left bound
      for (double i = -e_size / 2 + R_SIZE / 2; i <= e_size / 2 - R_SIZE / 2;
          i +=
          R_SIZE) {
        world_out << "    <model name='cinder_block_bound_" << n << "'>\n";
        world_out << "      <include>\n";
        world_out << "        <uri>model://cinder_block</uri>\n";
        world_out << "      </include>\n";
        world_out << "      <pose>" << -(e_size / 2 + R_SIZE / 4) << " " << i
            << " 0 0 0 " << M_PI_2 << "</pose>\n";
        world_out << "      <static>1</static>\n";
        world_out << "    </model>\n";
        n++;
      }

      n = 1;
      // Obstacles
      for (std::list<PolygonPtr>::iterator o = obstacles.begin();
          o != obstacles.end(); o++) {
        PointPtr p = (boost::static_pointer_cast<Cell>(*o))->get_center();
        int c = 1;
        for (double i = p->y - R_SIZE * 3 / 4; i <= p->y + R_SIZE * 3 / 4;
            i += R_SIZE / 2) {
          world_out << "    <model name='cinder_block_obstacle_" << n << "_"
              << c << "'>\n";
          world_out << "      <include>\n";
          world_out << "        <uri>model://cinder_block</uri>\n";
          world_out << "      </include>\n";
          world_out << "      <pose>" << p->x - R_SIZE / 2 << " " << i
              << " 0 0 0 0</pose>\n";
          world_out << "      <static>1</static>\n";
          world_out << "    </model>\n";
          c++;
        }
        for (double i = p->y - R_SIZE * 3 / 4; i <= p->y + R_SIZE * 3 / 4;
            i += R_SIZE / 2) {
          world_out << "    <model name='cinder_block_obstacle_" << n << "_"
              << c << "'>\n";
          world_out << "      <include>\n";
          world_out << "        <uri>model://cinder_block</uri>\n";
          world_out << "      </include>\n";
          world_out << "      <pose>" << p->x + R_SIZE / 2 << " " << i
              << " 0 0 0 0</pose>\n";
          world_out << "      <static>1</static>\n";
          world_out << "    </model>\n";
          c++;
        }
        n++;
      }
    }
  }
  world_in.close();
  world_out.close();

  environment = EnvironmentPtr(new Environment(space, obstacles));
  if (argc >= 4) {
    if (std::string(argv[3]) == "spiral_stc") {
      SpiralStcPtr plan_spiral_stc = SpiralStcPtr(new SpiralStc());
      plan_spiral_stc->initialize(starting_point, R_SIZE);
      tmp_path.insert(tmp_path.end(), starting_point);
      plan_spiral_stc->set_behavior_go_to(boost::bind(&test_go_to, _1, _2));
      plan_spiral_stc->set_behavior_see_obstacle(
          boost::bind(&test_see_obstacle, _1, _2));
      plan_spiral_stc->cover();
    } else if (std::string(argv[3]) == "full_spiral_stc") {
      FullSpiralStcPtr plan_full_spiral_stc = FullSpiralStcPtr(
          new FullSpiralStc());
      plan_full_spiral_stc->initialize(starting_point, R_SIZE);
      tmp_path.insert(tmp_path.end(), starting_point);
      plan_full_spiral_stc->set_behavior_go_to(
          boost::bind(&test_go_to, _1, _2));
      plan_full_spiral_stc->set_behavior_see_obstacle(
          boost::bind(&test_see_obstacle, _1, _2));
      plan_full_spiral_stc->cover();
    }
  }
  run(argc, argv);
  return 0;
}
