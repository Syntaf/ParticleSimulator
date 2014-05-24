#ifndef PARTICLE_H_
#define PARTICLE_H_

#include <glm/glm.hpp>
#include <vector>

class Particle
{
	public:
		Particle();
		Particle(float mass, float size, float life, float cameradistance,
				 unsigned char r, unsigned char g, unsigned char b, unsigned char a);

		void addForce(glm::vec3 const& m_force);
		void setSpeed(glm::vec3 const& speed);
		void setPosition(glm::vec3 const& pos);
		void setMass(float mass);
		void setSize(float size);
		void setLife(float life);
		void setCameradistance(float cameradistance);
		void setRGBA(unsigned char r, 
					 unsigned char g, 
					 unsigned char b,
					 unsigned char a);
		
		void getRGBA(char* RGBA);
		glm::vec3 getTotalForce() const;
		std::vector<glm::vec3> getForce() const;
		glm::vec3 getPosition() const;
		glm::vec3 getSpeed() const;
		float getMass() const;
		float getLife() const;
		float getCameradistance() const;
		float getSize() const;

		void updateParticle(float const& dt, glm::vec3 const& cameraPosition); 

		bool operator<(const Particle& that) const {
			return this->d_cameradistance > that.d_cameradistance;
		}
	private:
		std::vector<glm::vec3> d_force;		//force vector
		glm::vec3 d_pos;					//position of particle
		glm::vec3 d_speed;					//current speed of particle
		unsigned char d_r,d_g,d_b,d_a;		//color value
		float d_mass;						//mass of particle used for F=MA
		float d_size;						//size on screen
		float d_life;						//lifetime of particle
		float d_cameradistance;				//squared distance to camera, -1.0 if dead
};

#endif