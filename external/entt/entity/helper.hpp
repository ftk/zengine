#ifndef ENTT_ENTITY_HELPER_HPP
#define ENTT_ENTITY_HELPER_HPP


#include <type_traits>
#include "../core/hashed_string.hpp"
#include "../signal/sigh.hpp"
#include "registry.hpp"


namespace entt {


/**
 * @brief Dependency function prototype.
 *
 * A _dependency function_ is a built-in listener to use to automatically assign
 * components to an entity when a type has a dependency on some other types.
 *
 * This is a prototype function to use to create dependencies.<br/>
 * It isn't intended for direct use, although nothing forbids using it freely.
 *
 * @tparam Entity A valid entity type (see entt_traits for more details).
 * @tparam Component Types of components to assign to an entity if triggered.
 * @param registry A valid reference to a registry.
 * @param entity A valid entity identifier.
 */
template<typename Entity, typename... Component>
void dependency(registry<Entity> &registry, const Entity entity) {
    ((registry.template has<Component>(entity) ? void() : (registry.template assign<Component>(entity), void())), ...);
}


/**
 * @brief Connects a dependency function to the given sink.
 *
 * A _dependency function_ is a built-in listener to use to automatically assign
 * components to an entity when a type has a dependency on some other types.
 *
 * The following adds components `a_type` and `another_type` whenever `my_type`
 * is assigned to an entity:
 * @code{.cpp}
 * entt::registry registry;
 * entt::connect<a_type, another_type>(registry.construction<my_type>());
 * @endcode
 *
 * @tparam Dependency Types of components to assign to an entity if triggered.
 * @tparam Entity A valid entity type (see entt_traits for more details).
 * @param sink A sink object properly initialized.
 */
template<typename... Dependency, typename Entity>
inline void connect(sink<void(registry<Entity> &, const Entity)> sink) {
    sink.template connect<dependency<Entity, Dependency...>>();
}


/**
 * @brief Disconnects a dependency function from the given sink.
 *
 * A _dependency function_ is a built-in listener to use to automatically assign
 * components to an entity when a type has a dependency on some other types.
 *
 * The following breaks the dependency between the component `my_type` and the
 * components `a_type` and `another_type`:
 * @code{.cpp}
 * entt::registry registry;
 * entt::disconnect<a_type, another_type>(registry.construction<my_type>());
 * @endcode
 *
 * @tparam Dependency Types of components used to create the dependency.
 * @tparam Entity A valid entity type (see entt_traits for more details).
 * @param sink A sink object properly initialized.
 */
template<typename... Dependency, typename Entity>
inline void disconnect(sink<void(registry<Entity> &, const Entity)> sink) {
    sink.template disconnect<dependency<Entity, Dependency...>>();
}


/**
 * @brief Alias template to ease the assignment of labels to entities.
 *
 * If used in combination with hashed strings, it simplifies the assignment of
 * labels to entities and the use of labels in general where a type would be
 * required otherwise.<br/>
 * As an example and where the user defined literal for hashed strings hasn't
 * been changed:
 * @code{.cpp}
 * entt::registry registry;
 * registry.assign<entt::label<"enemy"_hs>>(entity);
 * @endcode
 *
 * @tparam Value The numeric representation of an instance of hashed string.
 */
template<typename hashed_string::hash_type Value>
using label = std::integral_constant<typename hashed_string::hash_type, Value>;


}


#endif // ENTT_ENTITY_HELPER_HPP
