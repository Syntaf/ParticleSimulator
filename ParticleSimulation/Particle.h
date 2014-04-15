#ifndef PARTICLE_H_
#define PARTICLE_H_

#include <SFML/Graphics.hpp>
using namespace std;
using namespace sf;

class Particle
{
	public:
		Particle();
		Particle(float mass, Vector2f position, Vector2f speed);

		float getMass() const;
		void setMass(float const& mass);

		vector<Vector2f> getForce() const;
		Vector2f getTotalForce() const;
		void addForce(Vector2f const& force);
		void clearForce();
		Vector2f getPosition() const;
		void setPosition(Vector2f const& position);
		Vector2f getSpeed() const;
		void setSpeed(Vector2f const& speed);
		Vector2f getAcceleration() const;
		void setAcceleration(Vector2f const& acceleration);
		void setStatic();
		void setMovable();
		bool isMovable() const;

		//most important function
		void updatePosition(float const& dt);
		
	private:
		float m_mass;

		vector<Vector2f> m_force;			//force applied to particle
		Vector2f m_position;				//position of particle
		Vector2f m_speed;					//speed of particle (v = ( x(t+dt) - x(t) )
		Vector2f m_acceleration;			//acceleration of particle (v(t+dt) - v(t))

		bool m_movable;

};


#endif