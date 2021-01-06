//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"
#include "SteeringAgent.h"
#include "CombinedBehaviours/TheFlock.h"

//SEEK
//****
SteeringOutput Seek::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	const float arrivalRadius{1.f};

	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();
	const float distance = steering.LinearVelocity.Magnitude();

	if (distance < arrivalRadius)
	{
		pAgent->SetLinearVelocity(Elite::ZeroVector2);
		return SteeringOutput{ Elite::Vector2{0,0} , pAgent->GetAngularVelocity() };
	}

	steering.LinearVelocity = (m_Target).Position - pAgent->GetPosition(); //Desired velocity
	steering.LinearVelocity.Normalize(); //Normalize
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed(); //Rescale to max speed

	//DEBUG RENDERING
	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0,0.5f }, 0.40f);
	}

	return steering;
}

//WANDER (base> SEEK)
//******
SteeringOutput Wander::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	pAgent->SetAutoOrient(true);
	Elite::Vector2 circlePos{ cos(pAgent->GetRotation() - ToRadians(90)) * m_Offset + pAgent->GetPosition().x ,
		sin(pAgent->GetRotation() - ToRadians(90)) * m_Offset + pAgent->GetPosition().y};

	m_WanderAngle += ((rand() % int(m_AngleChange * 200)) - int(m_AngleChange * 100)) / 100.f;;

	Elite::Vector2 targetPos{(float)cos(m_WanderAngle) * m_Radius, (float)sin(m_WanderAngle) * m_Radius};
	targetPos += circlePos;

	m_Target = targetPos;
	
	SteeringOutput steering{Seek::CalculateSteering(deltaT , pAgent)};

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawPoint( circlePos , 5, { 0,1,0,0.5f }, 0.40f);
		DEBUGRENDERER2D->DrawCircle(circlePos, m_Radius, { 1.f,0,0,0.5f }, 0.40f);
		DEBUGRENDERER2D->DrawPoint(targetPos, 5, { 0.f,0,1.f,0.5f }, 0.40f);
	}

	return steering;
}

//Flee (base> SEEK)
//******
SteeringOutput Flee::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};
	float distance{Distance(pAgent->GetPosition() , m_Target.Position)};
	
	steering = Seek::CalculateSteering(deltaT, pAgent);
	steering.LinearVelocity *= -1;
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed() * (m_FleeRadius - distance);
	return steering;
}

//ARRIVE
//****
SteeringOutput Arrive::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	const float arrivalRadius = 1.f;
	const float slowRadius = 3.f;

	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();
	const float distance = steering.LinearVelocity.Magnitude();

	if (distance < arrivalRadius)
	{
		pAgent->SetLinearVelocity(Elite::ZeroVector2);
		return SteeringOutput{ Elite::Vector2{0,0} ,0 };
	}
	steering.LinearVelocity.Normalize(); //lengte van vector = 1
	if (distance < slowRadius)
	{
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed() * (distance / slowRadius); //vertragen
	}
	else
	{
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();
	}
	pAgent->SetLinearVelocity(steering.LinearVelocity);

	//DEBUG RENDERING
	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0,0.5f }, 0.40f);
	}

	return steering;
}

//Face
//******
SteeringOutput Face::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};
	pAgent->SetAutoOrient(false);

	Elite::Vector2 destVect{ m_Target.Position - pAgent->GetPosition() };

	float targetAngle{atan2(destVect.GetNormalized().y , destVect.GetNormalized().x)};
	targetAngle *= (float(180.f / M_PI)) ;

	float currentAngle{(pAgent->GetRotation() * float(180.f / M_PI)) -90.f };
	float rotation{targetAngle - currentAngle};

	if (rotation > 10.f)
	{
		return SteeringOutput{ Elite::Vector2{0,0} , 10.f };
	}
	else if (rotation < -10.f)
	{
		return SteeringOutput{ Elite::Vector2{0,0} , -10.f };
	}
	else
	{
		return SteeringOutput{ Elite::Vector2{0,0} , rotation };
	}
}

//Pursuit (base> SEEK)
//******
SteeringOutput Pursuit::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	(m_Target).Position += m_Target.LinearVelocity;

	return  Seek::CalculateSteering(deltaT , pAgent);

	//DEBUG RENDERING
	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(),m_Target.Position , 5, { 0,1,0,0.5f }, 0.40f);
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), pAgent->GetLinearVelocity(), 5, { 1,0,0,0.5f }, 0.40f);
	}
}
//Evade (base> FLEE)
//******
SteeringOutput Evade::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	if (Distance(pAgent->GetPosition() , m_Target.Position) <= m_Radius)
	{
		(m_Target).Position += m_Target.LinearVelocity;
		steering = Flee::CalculateSteering(deltaT, pAgent);
		steering.IsValid = true;
		return  steering;
	}
	steering.IsValid = false;
	return steering;

	//DEBUG RENDERING
	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), m_Target.Position, 5, { 0,1,0,0.5f }, 0.40f);
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), pAgent->GetLinearVelocity(), 5, { 1,0,0,0.5f }, 0.40f);
	}
}

void Evade::SetRadius(float radius)
{
	m_Radius = radius;
}


