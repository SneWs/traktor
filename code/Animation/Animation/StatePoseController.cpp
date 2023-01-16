/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Joint.h"
#include "Animation/Skeleton.h"
#include "Animation/SkeletonUtils.h"
#include "Animation/Animation/Animation.h"
#include "Animation/Animation/ITransformTime.h"
#include "Animation/Animation/StateNodeAnimation.h"
#include "Animation/Animation/StateGraph.h"
#include "Animation/Animation/StatePoseController.h"
#include "Animation/Animation/Transition.h"
#include "Core/Math/Const.h"
#include "Core/Math/Random.h"

namespace traktor::animation
{
	namespace
	{

Random s_random;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.StatePoseController", StatePoseController, IPoseController)

StatePoseController::StatePoseController(const resource::Proxy< StateGraph >& stateGraph, ITransformTime* transformTime)
:	m_stateGraph(stateGraph)
,	m_transformTime(transformTime)
,	m_blendState(0.0f)
,	m_blendDuration(0.0f)
,	m_timeFactor(1.0f)
{
}

bool StatePoseController::setState(const std::wstring& stateName)
{
	if (!m_stateGraph)
		return false;

	if (m_currentState && m_currentState->getName() == stateName)
		return true;

	for (auto state : m_stateGraph->getStates())
	{
		if (state->getName() == stateName)
		{
			m_currentState = state;
			m_currentState->prepareContext(m_currentStateContext);
			m_nextState = nullptr;
			m_blendState = 0.0f;
			m_blendDuration = 0.0f;
			return true;
		}
	}

	return false;
}

void StatePoseController::setCondition(const std::wstring& condition, bool enabled, bool reset)
{
	m_conditions[condition].first = enabled;
	m_conditions[condition].second = reset;
}

void StatePoseController::setTime(float time)
{
	T_FATAL_ASSERT (m_currentState);
	m_currentStateContext.setTime(time);
	m_currentStateContext.setIndexHint(-1);
}

float StatePoseController::getTime() const
{
	return m_currentStateContext.getTime();
}

void StatePoseController::setTimeFactor(float timeFactor)
{
	m_timeFactor = timeFactor;
}

float StatePoseController::getTimeFactor() const
{
	return m_timeFactor;
}

void StatePoseController::destroy()
{
}

void StatePoseController::setTransform(const Transform& transform)
{
}

bool StatePoseController::evaluate(
	float time,
	float deltaTime,
	const Transform& worldTransform,
	const Skeleton* skeleton,
	const AlignedVector< Transform >& jointTransforms,
	AlignedVector< Transform >& outPoseTransforms
)
{
	const float wallDeltaTime = deltaTime;
	bool continous = true;

	if (!skeleton)
		return false;

	if (m_stateGraph.changed())
	{
		m_currentState = nullptr;
		m_stateGraph.consume();
	}

	// Prepare graph evaluation context.
	if (!m_currentState)
	{
		m_currentState = m_stateGraph->getRootState();
		if (m_currentState)
		{
			if (!m_currentState->prepareContext(m_currentStateContext))
				return false;
		}
		m_nextState = nullptr;
		m_blendState = 0.0f;
		m_blendDuration = 0.0f;
	}

	if (!m_currentState)
		return false;

	// Transform, or remap, time.
	if (m_transformTime && is_a< StateNodeAnimation >(m_currentState))
	{
		const Animation* animation = static_cast< const StateNodeAnimation* >(m_currentState.c_ptr())->getAnimation();
		if (animation)
			m_transformTime->calculateTime(animation, worldTransform, time, deltaTime);
	}

	// Evaluate current state.
	m_currentState->evaluate(
		m_currentStateContext,
		m_evaluatePose
	);
	m_currentStateContext.setTime(m_currentStateContext.getTime() + deltaTime * m_timeFactor);

	// Build final pose transforms.
	if (m_nextState)
	{
		// Only blend between states if there is a transition time.
		if (m_blendDuration > 0.0f)
		{
			Pose nextPose, blendPose;

			// Transform, or remap, time.
			if (m_transformTime && is_a< StateNodeAnimation >(m_nextState))
			{
				const Animation* animation = static_cast< const StateNodeAnimation* >(m_nextState.c_ptr())->getAnimation();
				if (animation)
					m_transformTime->calculateTime(animation, worldTransform, time, deltaTime);
			}

			m_nextState->evaluate(
				m_nextStateContext,
				nextPose
			);
			m_nextStateContext.setTime(m_nextStateContext.getTime() + deltaTime * m_timeFactor);

			const Scalar blend = Scalar(sinf((m_blendState / m_blendDuration) * PI / 2.0f));

			blendPoses(
				&m_evaluatePose,
				&nextPose,
				blend,
				&blendPose
			);

			calculatePoseTransforms(
				skeleton,
				&blendPose,
				outPoseTransforms
			);
		}
		else
		{
			calculatePoseTransforms(
				skeleton,
				&m_evaluatePose,
				outPoseTransforms
			);
		}

		// Swap in next state when we've completely blended into it.
		m_blendState += wallDeltaTime;
		if (m_blendState >= m_blendDuration)
		{
			m_currentState = m_nextState;
			m_currentStateContext = m_nextStateContext;
			m_nextState = nullptr;
			m_blendState = 0.0f;
			m_blendDuration = 0.0f;
			continous = bool(m_blendDuration > FUZZY_EPSILON);
		}
	}
	else
	{
		calculatePoseTransforms(
			skeleton,
			&m_evaluatePose,
			outPoseTransforms
		);
	}

	// Execute transition to another state.
	if (!m_nextState)
	{
		const RefArray< Transition >& transitions = m_stateGraph->getTransitions();
		Transition* selectedTransition = nullptr;

		// First try all transitions with explicit condition.
		for (auto transition : transitions)
		{
			if (transition->from() != m_currentState || transition->getCondition().empty())
				continue;

			// Is transition permitted?
			bool transitionPermitted = false;
			switch (transition->getMoment())
			{
			case Transition::Moment::Immediatly:
				transitionPermitted = true;
				break;

			case Transition::Moment::End:
				{
					const float timeLeft = max(m_currentStateContext.getDuration() - m_currentStateContext.getTime(), 0.0f);
					if (timeLeft <= transition->getDuration())
						transitionPermitted = true;
				}
				break;
			}
			if (!transitionPermitted)
				continue;

			// Is condition satisfied?
			bool value = false;
			const std::wstring& condition = transition->getCondition();
			if (condition[0] == L'!')
			{
				std::pair< bool, bool >& cv = m_conditions[condition.substr(1)];
				value = !cv.first;
				if (value && cv.second)
				{
					cv.first = !cv.first;
					cv.second = false;
				}
			}
			else
			{
				std::pair< bool, bool >& cv = m_conditions[condition];
				value = cv.first;
				if (value && cv.second)
				{
					cv.first = !cv.first;
					cv.second = false;
				}
			}
			if (!value)
				continue;

			// Found valid transition.
			selectedTransition = transition;
			break;
		}

		// Still no transition state found, we try all transitions without explicit condition.
		if (selectedTransition == nullptr)
		{
			RefArray< Transition > candidateTransitions;
			for (auto transition : transitions)
			{
				if (transition->from() != m_currentState || !transition->getCondition().empty())
					continue;

				// Is transition permitted?
				bool transitionPermitted = false;
				switch (transition->getMoment())
				{
				case Transition::Moment::Immediatly:
					transitionPermitted = true;
					break;

				case Transition::Moment::End:
					{
						const float timeLeft = max(m_currentStateContext.getDuration() - m_currentStateContext.getTime(), 0.0f);
						if (timeLeft <= transition->getDuration())
							transitionPermitted = true;
					}
					break;
				}
				if (!transitionPermitted)
					continue;

				candidateTransitions.push_back(transition);
			}

			// Randomly select one of the found, valid, transitions.
			if (!candidateTransitions.empty())
			{
				const uint32_t i = s_random.next() % candidateTransitions.size();
				selectedTransition = candidateTransitions[i];
			}
		}

		// Begin transition to found state.
		if (selectedTransition != nullptr)
		{
			m_nextState = selectedTransition->to();
			m_nextState->prepareContext(m_nextStateContext);
			m_blendState = 0.0f;
			m_blendDuration = selectedTransition->getDuration();
		}
	}

	return continous;
}

void StatePoseController::estimateVelocities(
	const Skeleton* skeleton,
	AlignedVector< Velocity >& outVelocities
)
{
}

}
