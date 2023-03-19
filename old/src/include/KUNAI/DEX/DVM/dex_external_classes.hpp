/**
 * @file dex_external_classes.hpp
 * @author @Farenain
 *
 * @brief Android external class used to create specific
 *        object in the analysis of the code, for calls
 *        to external classes.
 */

#ifndef DEX_EXTERNAL_CLASSES_HPP
#define DEX_EXTERNAL_CLASSES_HPP

#include <iostream>
#include <vector>
#include <memory>

#include "KUNAI/DEX/DVM/dex_external_methods.hpp"

namespace KUNAI
{
    namespace DEX
    {

        class ExternalClass;

        using externalclass_t = std::unique_ptr<ExternalClass>;

        class ExternalClass
        {
        public:
            /**
             * @brief Construct a new External Class object,
             *        representation of class out of DEX.
             *
             * @param name
             */
            ExternalClass(std::string name);

            /**
             * @brief Destroy the External Class object, clear methods.
             */
            ~ExternalClass() = default;

            /**
             * @brief Get Class name.
             *
             * @return std::string
             */
            std::string &get_name()
            {
                return name;
            }

            /**
             * @brief Get the number of methods in the class.
             *
             * @return std::uint64_t
             */
            std::uint64_t get_number_of_methods()
            {
                return methods.size();
            }

            /**
             * @brief Get the method by position.
             *
             * @param pos
             * @return ExternalMethod*
             */
            ExternalMethod *get_method_by_pos(std::uint64_t pos);

            /**
             * @brief Add external method to class.
             *
             * @param method
             */
            void add_method(ExternalMethod *method)
            {
                methods.push_back(method);
            }

        private:
            std::string name;
            std::vector<ExternalMethod *> methods;
        };
    }
}

#endif