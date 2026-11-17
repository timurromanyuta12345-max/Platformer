#include "EventManager.h"
#include "MemoryManager.h"
#include "Utilities.h"
#include <fstream>
#include <string_view>
#include <spanstream>
#include <cstdlib>

EventManager::EventManager()
	: m_flags{ true }
	, m_bindings{ LocalMemoryManager::getManager().getAlloc() } // std::pmr::polymorphic_allocator<std::pair<const std::string_view, PmrBindingPtr>> resource()
	, m_callbacks{ LocalMemoryManager::getManager().getAlloc() } // std::pmr::polymorphic_allocator<std::pair<const std::string_view, std::move_only_function<void(EventDetails*)>>> resource()
	, m_currentState{ BaseState::StateType(0) }
{
	m_bindings.max_load_factor(0.5f);
	m_callbacks.max_load_factor(0.5f);

	loadBindings();
}

bool EventManager::addBinding(PmrBindingPtr l_binding) {
	if (m_bindings.find(l_binding->m_name) != m_bindings.end()) [[unlikely]] {
		return false;
	}
	return m_bindings.emplace(l_binding->m_name, std::move(l_binding)).second;
}

bool EventManager::removeBinding(std::string_view l_name) {
	auto itr{ m_bindings.find(l_name) };

	if (itr == m_bindings.end()) [[unlikely]] {
		return false;
	}

	m_bindings.erase(itr);
	return true;
}

void EventManager::handleEvent(const sf::Event& l_event, sf::RenderWindow& l_window) {
	if (!m_flags[static_cast<size_t>(Flags::HasFocus)]) [[unlikely]] {
		return;
	}

	EventType l_sfmlType;
	int l_keyCode = -1;
	float l_delta;
	sf::Vector2u l_sizeEv;
	char32_t l_unicodeEv;

	if (const auto* keyPressed = l_event.getIf<sf::Event::KeyPressed>()) [[likely]] {
		l_sfmlType = EventType::KeyDown;
		l_keyCode = static_cast<int>(keyPressed->code);
	}
	else
		if (const auto* keyReleased = l_event.getIf<sf::Event::KeyReleased>()) [[likely]] {
			l_sfmlType = EventType::KeyUp;
			l_keyCode = static_cast<int>(keyReleased->code);
		}
		else
			if (const auto* mousePressed = l_event.getIf<sf::Event::MouseButtonPressed>()) [[likely]] {
				l_sfmlType = EventType::MButtonDown;
				l_keyCode = static_cast<int>(mousePressed->button);

			}
			else
				if (const auto* mouseReleased = l_event.getIf<sf::Event::MouseButtonReleased>()) [[likely]] {
					l_sfmlType = EventType::MButtonUp;
					l_keyCode = static_cast<int>(mouseReleased->button);
				}
				else
					if (const auto* mouseWheel = l_event.getIf<sf::Event::MouseWheelScrolled>()) [[likely]] {
						l_sfmlType = EventType::MouseWheel;
						l_delta = mouseWheel->delta;
					}
					else
						if (const auto* sizeEvent = l_event.getIf<sf::Event::Resized>()) [[unlikely]] {
							l_sfmlType = EventType::WindowResized;
							l_sizeEv = sizeEvent->size;
						}
						else
							if (l_event.is<sf::Event::FocusGained>()) [[unlikely]] {
								l_sfmlType = EventType::GainedFocus;
							}
							else
								if (l_event.is<sf::Event::FocusLost>()) [[unlikely]] {
									l_sfmlType = EventType::LostFocus;
								}
								else
									if (l_event.is<sf::Event::MouseEntered>()) [[unlikely]] {
										l_sfmlType = EventType::MouseEntered;
									}
									else
										if (l_event.is<sf::Event::MouseLeft>()) [[unlikely]] {
											l_sfmlType = EventType::MouseLeft;
										}
										else
											if (l_event.is<sf::Event::Closed>()) [[unlikely]] {
												l_sfmlType = EventType::Closed;
											}
											else
												if (const auto* textEvent = l_event.getIf<sf::Event::TextEntered>()) [[unlikely]] {
													l_sfmlType = EventType::TextEntered;
													l_unicodeEv = textEvent->unicode;
												}
												else {
													l_sfmlType = EventType::Keyboard;
												}

	for (auto& l_b_itr : m_bindings) {
		Binding* l_bind{ l_b_itr.second.get() };
		for (auto& l_e_itr : l_bind->m_events) {

			if (l_e_itr.first != l_sfmlType) [[unlikely]] {
				continue;
			}

			if (l_sfmlType == EventType::KeyDown or l_sfmlType == EventType::KeyUp) [[likely]] {
				if (std::get<int>(l_e_itr.second.m_code) == l_keyCode or std::get<int>(l_e_itr.second.m_code) == -1) [[likely]] {
					if (std::get<int>(l_e_itr.second.m_code) != -1) [[likely]] {
						l_bind->m_details.m_keyCode = l_keyCode;
					}
					
					++(l_bind->m_c);
					continue;
				}
			}
			else
				if (l_sfmlType == EventType::MButtonDown or l_sfmlType == EventType::MButtonUp) [[likely]] {
					if (std::get<int>(l_e_itr.second.m_code) == l_keyCode or std::get<int>(l_e_itr.second.m_code) == -1) [[likely]] {
						l_bind->m_details.m_mouse = getMousePosition(&l_window);
						if (std::get<int>(l_e_itr.second.m_code) != -1) [[likely]] {
							l_bind->m_details.m_keyCode = l_keyCode;
						}

						++(l_bind->m_c);
						continue;
					}
				}
				else {
					if (l_sfmlType == EventType::MouseWheel) [[unlikely]] {
						l_bind->m_details.m_mouseWheelDelta = l_delta;
						++(l_bind->m_c);
					}
					else
						if (l_sfmlType == EventType::WindowResized) [[unlikely]] {
							l_bind->m_details.m_size = l_sizeEv;
							++(l_bind->m_c);
						}
						else
							if (l_sfmlType == EventType::TextEntered) [[unlikely]] {
								l_bind->m_details.m_textEntered = static_cast<uint32_t>(l_unicodeEv);
								++(l_bind->m_c);
							}
				}
		}
	}
}

void EventManager::update() {
	if (!m_flags[(static_cast<size_t>(Flags::HasFocus))]) [[unlikely]] {
		return;
	}

	for (auto& l_b_itr : m_bindings) {
		Binding* l_bind = l_b_itr.second.get();
			for (auto& l_e_itr : l_bind->m_events) {
				switch (l_e_itr.first) {
				case (EventType::Keyboard):
					if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key(std::get<int>(l_e_itr.second.m_code)))) [[likely]] {
						if (std::get<int>(l_e_itr.second.m_code) != -1) [[likely]] {
							l_bind->m_details.m_keyCode = std::get<int>(l_e_itr.second.m_code);
						}
						++(l_bind->m_c);
					}
					break;

				case (EventType::Mouse):
					if (sf::Mouse::isButtonPressed(sf::Mouse::Button(std::get<int>(l_e_itr.second.m_code)))) [[likely]] {
						if (std::get<int>(l_e_itr.second.m_code) != -1) [[likely]] {
							l_bind->m_details.m_keyCode = std::get<int>(l_e_itr.second.m_code);
						}
						++(l_bind->m_c);
					}
					break;

				case (EventType::Joystick):
					break;

				default:
					break;
				}
			}

			if (l_bind->m_events.size() == l_bind->m_c) [[likely]] {
				auto stateCallbacks{ m_callbacks.find(m_currentState) };
				auto otherCallbacks{ m_callbacks.find(BaseState::StateType(0)) };


				if (stateCallbacks != m_callbacks.end()) [[likely]] {
					auto callItr{ stateCallbacks->second.find(l_bind->m_name) };
					if (callItr != stateCallbacks->second.end()) [[unlikely]] {
						callItr->second(&l_bind->m_details);
					}
				}
				if (otherCallbacks != m_callbacks.end()) [[unlikely]] {
					auto callItr{ otherCallbacks->second.find(l_bind->m_name) };
					if (callItr != otherCallbacks->second.end()) {
						callItr->second(&l_bind->m_details);
					}
				}
			}
			
			l_bind->m_c = 0;
			l_bind->m_details.clear();
	}
}

void EventManager::loadBindings() {
	std::pmr::string tempForIfstream(utils::getWorkingDirectory() + "keys.cfg", LocalMemoryManager::getManager().getAlloc());
	std::ifstream l_bindings(tempForIfstream.c_str());
	if (!l_bindings) [[unlikely]] {
		exit(EXIT_FAILURE);
	}

	std::pmr::string line(LocalMemoryManager::getManager().getAlloc()), name(LocalMemoryManager::getManager().getAlloc()), token(LocalMemoryManager::getManager().getAlloc());

	while (std::getline(l_bindings, line)) {
		if (line.empty() || (line[0] == '/' && line[1] == '/')) [[unlikely]] {
			continue;
		}

		std::spanstream textStream(line);
		textStream >> name;
		PmrBindingPtr l_bind = LocalMemoryManager::getManager().allocateCustom(name);

		bool parseError{ false };

		while (textStream >> token) {
			size_t colon = token.find(':');
			if (colon == std::pmr::string::npos) [[unlikely]] {
				parseError = true;
				break;
			}

			int typeRaw, keyCode;
			std::string_view stringForFromChars(token);
			auto res1 = std::from_chars(stringForFromChars.data(), stringForFromChars.data() + colon, typeRaw);
			auto res2 = std::from_chars(stringForFromChars.data() + colon + 1, stringForFromChars.data() + stringForFromChars.size(), keyCode);

			if (res1.ec != std::errc{} || res2.ec != std::errc{}) [[unlikely]] {
				parseError = true;
				break;
			}

			EventType type = static_cast<EventType>(typeRaw);
			EventInfo eventInfo(keyCode);
			l_bind->bindEvent(type, eventInfo);
		}

		if (parseError) [[unlikely]] {
			exit(EXIT_FAILURE);
		}

		if (!addBinding(std::move(l_bind))) [[ unlikely]] {
			l_bind = nullptr;
		}

	}
	l_bindings.close();
}

bool EventManager::removeCallback(BaseState::StateType l_type, const std::string_view l_name) {
	auto itr{ m_callbacks.find(l_type) };
	if (itr == m_callbacks.end()) [[unlikely]] {
		return false;
	}

	auto itr2{ itr->second.find(l_name) };
	if (itr2 == itr->second.end()) {
		return false;
	}

	itr->second.erase(l_name);
	return true;
}