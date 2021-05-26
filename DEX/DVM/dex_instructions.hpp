/***
 * File: dex_instructions.hpp
 * Author: @Farenain
 * 
 * All the instructions from the dalvik machine
 * represented as classes.
 */

#ifndef DEX_INSTRUCTIONS_HPP
#define DEX_INSTRUCTIONS_HPP

#include <iostream>
#include <memory>
#include <fstream>
#include <tuple>
#include <vector>

#include "dex_dalvik_opcodes.hpp"
#include "dex_dvm_types.hpp"
#include "utils.hpp"
#include "exceptions.hpp"

namespace KUNAI {
    namespace DEX {
        
        class Instruction
        {
        public:
            Instruction(std::shared_ptr<DalvikOpcodes> dalvik_opcodes, std::ifstream& input_file);
            ~Instruction();

            DVMTypes::Kind get_kind();
            std::string get_translated_kind();
            std::string get_name();
            std::uint32_t get_length();
            std::uint32_t get_OP();
            std::shared_ptr<DalvikOpcodes> get_dalvik_opcodes();

            void set_length(std::uint32_t length);
            void set_OP(std::uint32_t OP);

            virtual void show_instruction();
            virtual std::string get_output();
            virtual std::uint64_t get_raw();
            virtual std::vector<std::tuple<DVMTypes::Operand, std::uint64_t>> get_operands();
        private:
            std::shared_ptr<DalvikOpcodes> dalvik_opcodes;
            std::uint32_t length;
            std::uint32_t OP;
        };

        class Instruction00x : public Instruction
        {
        public:
            Instruction00x(std::shared_ptr<DalvikOpcodes> dalvik_opcodes, std::ifstream& input_file);
            ~Instruction00x();
        };

        class Instruction10x : public Instruction
        /***
         * Waste cycles. 
         * 
         * Example of instruction:
         * 
         * nop
         * 
         * length = 2 bytes
         */
        {
        public:
            Instruction10x(std::shared_ptr<DalvikOpcodes> dalvik_opcodes, std::ifstream& input_file);
            ~Instruction10x();

            virtual std::uint64_t get_raw();
        };
        
        class Instruction12x : public Instruction
        /***
         * Move the contents of one non-object register to another.
         * 
         * Example of instruction:
         * 
         * move vA, vB
         * 
         * vA: dest reg (4 bits)
         * vB: src reg (4 bits)
         */
        {
        public:
            Instruction12x(std::shared_ptr<DalvikOpcodes> dalvik_opcodes, std::ifstream& input_file);
            ~Instruction12x();

            virtual std::string get_output();
            virtual std::uint64_t get_raw();
            virtual std::vector<std::tuple<DVMTypes::Operand, std::uint64_t>> get_operands();

            DVMTypes::Operand get_source_type();
            std::uint8_t get_source();
            DVMTypes::Operand get_destination_type();
            std::uint8_t get_destination();
        private:
            std::uint8_t vA; // destination
            std::uint8_t vB; // source
        };

        class Instruction22x : public Instruction
        /***
         * Move the contents of one non-object register to another.
         * 
         * Example of instruction:
         * 
         * move/from16 vAA, vBBBB
         * 
         * vAA: destination register (8 bits)
         * vBBBB: source register (16 bits)
         */
        {
        public:
            Instruction22x(std::shared_ptr<DalvikOpcodes> dalvik_opcodes, std::ifstream& input_file);
            ~Instruction22x();

            virtual std::string get_output();
            virtual std::uint64_t get_raw();
            virtual std::vector<std::tuple<DVMTypes::Operand, std::uint64_t>> get_operands();

            DVMTypes::Operand get_source_type();
            std::uint16_t get_source();
            DVMTypes::Operand get_destination_type();
            std::uint8_t get_destination();
        private:
            std::uint8_t vAA;
            std::uint16_t vBBBB;
        };
        
        class Instruction32x : public Instruction
        /***
         * Move the contents of one non-object register to another.
         * 
         * Example of instruction:
         * 
         * move/16 vAAAA, vBBBB
         * 
         * vAAAA: destination register (16 bits)
         * vBBBB: source register (16 bits)
         */
        {
        public:
            Instruction32x(std::shared_ptr<DalvikOpcodes> dalvik_opcodes, std::ifstream& input_file);
            ~Instruction32x();

            virtual std::string get_output();
            virtual std::uint64_t get_raw();
            virtual std::vector<std::tuple<DVMTypes::Operand, std::uint64_t>> get_operands();

            DVMTypes::Operand get_source_type();
            std::uint16_t get_source();
            DVMTypes::Operand get_destination_type();
            std::uint16_t get_destination();
        private:
            std::uint16_t vAAAA;
            std::uint16_t vBBBB;
        };

        class Instruction11x : public Instruction
        /***
         * Move single, double-words or object from invoke results, also
         * save caught exception into given register.
         * 
         * Example of instruction:
         * 
         * move-result vAA
         * 
         * vAA: destination register (8 bits)
         */
        {
        public:
            Instruction11x(std::shared_ptr<DalvikOpcodes> dalvik_opcodes, std::ifstream& input_file);
            ~Instruction11x();

            virtual std::string get_output();
            virtual std::uint64_t get_raw();
            virtual std::vector<std::tuple<DVMTypes::Operand, std::uint64_t>> get_operands();

            DVMTypes::Operand get_destination_type();
            std::uint8_t get_destination();
        private:
            std::uint8_t vAA;
        };
    
        class Instruction11n : public Instruction
        /***
         * Move the given literal value
         * 
         * Example Instruction:
         * 
         * const/4 vA, #+B
         * 
         * vA: destination register (4 bits)
         * #+B: signed int (4 bits)
         */
        {
        public:
            Instruction11n(std::shared_ptr<DalvikOpcodes> dalvik_opcodes, std::ifstream& input_file);
            ~Instruction11n();

            virtual std::string get_output();
            virtual std::uint64_t get_raw();
            virtual std::vector<std::tuple<DVMTypes::Operand, std::uint64_t>> get_operands();

            DVMTypes::Operand get_source_type();
            std::int8_t get_source();
            DVMTypes::Operand get_destination_type();
            std::uint8_t get_destination();
        private:
            std::uint8_t vA;
            std::int8_t nB;
        };

        class Instruction21s : public Instruction
        /***
         * Move given literal value into specified register.
         * 
         * Example Instruction:
         * 
         * const/16 vAA, #+BBBB
         * 
         * vAA: destination register (8 bits)
         * #+BBBB: signed int (16 bits)
         */
        {
        public:
            Instruction21s(std::shared_ptr<DalvikOpcodes> dalvik_opcodes, std::ifstream& input_file);
            ~Instruction21s();

            virtual std::string get_output();
            virtual std::uint64_t get_raw();
            virtual std::vector<std::tuple<DVMTypes::Operand, std::uint64_t>> get_operands();

            DVMTypes::Operand get_source_type();
            std::int16_t get_source();
            DVMTypes::Operand get_destination_type();
            std::uint8_t get_destination();
        private:
            std::uint8_t vA;
            std::int16_t nBBBB;
        };

        class Instruction31i : public Instruction
        /***
         * Move given literal value into specified register
         * 
         * Example Instruction:
         * 
         * const vAA, #+BBBBBBBB
         * 
         * vAA: destination register (8 bits)
         * #+BBBBBBBB: arbitrary 32-bit constant
         */
        {
        public:
            Instruction31i(std::shared_ptr<DalvikOpcodes> dalvik_opcodes, std::ifstream& input_file);
            ~Instruction31i();

            virtual std::string get_output();
            virtual std::uint64_t get_raw();
            virtual std::vector<std::tuple<DVMTypes::Operand, std::uint64_t>> get_operands();

            DVMTypes::Operand get_source_type();
            std::int32_t get_source();
            DVMTypes::Operand get_destination_type();
            std::uint8_t get_destination();
        private:
            std::uint8_t vAA;
            std::uint32_t nBBBBBBBB;   
        };

        class Instruction21h : public Instruction
        /***
         * Move given literal value into specified register.
         * 
         * Example Instruction:
         * 
         * const/high16 vAA, #+BBBB0000
         * 
         * vAA: destination register (8 bits).
         * #+BBBB0000: signed int (16 bits)
         */
        {
        public:
            Instruction21h(std::shared_ptr<DalvikOpcodes> dalvik_opcodes, std::ifstream& input_file);
            ~Instruction21h();

            virtual std::string get_output();
            virtual std::uint64_t get_raw();
            virtual std::vector<std::tuple<DVMTypes::Operand, std::uint64_t>> get_operands();

            DVMTypes::Operand get_source_type();
            std::int64_t get_source();
            DVMTypes::Operand get_destination_type();
            std::uint8_t get_destination();
        private:
            std::uint8_t vAA;
            std::int64_t nBBBB;
            std::int16_t nBBBB_aux;
        };

        class Instruction51l : public Instruction
        /***
         * Move given literal value into specified register pair
         * 
         * Example Instruction:
         * 
         * const-wide vAA, #+BBBBBBBBBBBBBBBB
         * 
         * vAA: destination register (8 bits)
         * #+BBBBBBBBBBBBBBBB: arbitrary double-width constant (64 bits)
         */
        {
        public:
            Instruction51l(std::shared_ptr<DalvikOpcodes> dalvik_opcodes, std::ifstream& input_file);
            ~Instruction51l();

            virtual std::string get_output();
            virtual std::uint64_t get_raw();
            virtual std::vector<std::tuple<DVMTypes::Operand, std::uint64_t>> get_operands();

            DVMTypes::Operand get_source_type();
            std::uint64_t get_source();
            DVMTypes::Operand get_destination_type();
            std::uint8_t get_destination();
        private:
            std::uint8_t vAA;
            std::uint64_t nBBBBBBBBBBBBBBBB;
        };

        class Instruction21c : public Instruction
        /***
         * Move a reference to register from a string, type, etc.
         * 
         * Example instruction:
         * 
         * const-string vAA, string@BBBB
         * 
         * vAA: destination register (8 bits)
         * string/type@BBBB: string/type index (16 bits)
         */
        {
        public:
            Instruction21c(std::shared_ptr<DalvikOpcodes> dalvik_opcodes, std::ifstream& input_file);
            ~Instruction21c();

            virtual std::string get_output();
            virtual std::uint64_t get_raw();
            virtual std::vector<std::tuple<DVMTypes::Operand, std::uint64_t>> get_operands();

            DVMTypes::Operand get_source_type();
            std::uint16_t get_source();

            DVMTypes::Kind get_source_kind();
            std::string* get_source_str();
            Type* get_source_typeid();
            FieldID* get_source_static_field();
            MethodID* get_source_method();
            ProtoID* get_source_proto();

            DVMTypes::Operand get_destination_type();
            std::uint8_t get_destination();
        private:
            std::uint8_t vAA;
            std::uint16_t iBBBB;
        };

        class Instruction31c : public Instruction
        /***
         * Move a reference to the string specified by the given index into the specified register.
         * 
         * Example Instruction:
         * 
         * const-string/jumbo vAA, string@BBBBBBBB
         * 
         * vAA: destination register (8 bits).
         * string@BBBBBBBB: string index (32 bits)
         */
        {
        public:
            Instruction31c(std::shared_ptr<DalvikOpcodes> dalvik_opcodes, std::ifstream& input_file);
            ~Instruction31c();

            virtual std::string get_output();
            virtual std::uint64_t get_raw();
            virtual std::vector<std::tuple<DVMTypes::Operand, std::uint64_t>> get_operands();

            DVMTypes::Operand get_source_type();
            std::uint16_t get_source();

            DVMTypes::Kind get_source_kind();
            std::string* get_source_str();

            DVMTypes::Operand get_destination_type();
            std::uint8_t get_destination();
        private:
            std::uint8_t vAA;
            std::uint32_t iBBBBBBBB;
        };

        class Instruction22c : public Instruction
        /***
         * Store in the given destination register 1 if the indicated 
         * reference is an instance of the given type/field, or 0 if not. 
         * 
         * Example Instruction:
         * 
         * instance-of vA, vB, type@CCCC
         * 
         * vA: destination register (4 bits).
         * vB: reference-bearing register (4 bits).
         * field/type@CCCC: field/type index (16 bits)
         */
        {
        public:
            Instruction22c(std::shared_ptr<DalvikOpcodes> dalvik_opcodes, std::ifstream& input_file);
            ~Instruction22c();

            virtual std::string get_output();
            virtual std::uint64_t get_raw();
            virtual std::vector<std::tuple<DVMTypes::Operand, std::uint64_t>> get_operands();

            DVMTypes::Operand get_first_operand_type();
            std::uint8_t get_first_operand();

            DVMTypes::Operand get_second_operand_type();
            std::uint8_t get_second_operand();

            DVMTypes::Operand get_third_operand_type();
            std::uint16_t get_third_operand();
            DVMTypes::Kind get_third_operand_kind();
            Type* get_third_operand_typeId();
            FieldID* get_third_operand_FieldId();
        private:
            std::uint8_t vA;
            std::uint8_t vB;
            std::uint16_t iCCCC;
        };

        class Instruction35c : public Instruction
        /***
         * Construct array of given type and size, filling it with supplied
         * contents. Type must be an array type. Array's contents must be
         * single-word.
         * 
         * Example instruction:
         * 
         *  filled-new-array {vC, vD, vE, vF, vG}, type@BBBB
         * 
         * A: array size and argument word count (4 bits).
         * B: type index (16 bits).
         * C..G: argument registers (4 bits each).
         */ 
        {
            Instruction35c(std::shared_ptr<DalvikOpcodes> dalvik_opcodes, std::ifstream& input_file);
            ~Instruction35c();

            virtual std::string get_output();
            virtual std::uint64_t get_raw();
            virtual std::vector<std::tuple<DVMTypes::Operand, std::uint64_t>> get_operands();

            std::uint8_t get_array_size();

            std::uint16_t get_type_index();

            DVMTypes::Operand get_operands_types();
            Type* get_operands_kind();
            std::string get_operands_kind_str();
            std::uint8_t get_operand_register(std::uint8_t index);

        private:
            std::uint8_t array_size;
            std::uint16_t type_index;
            std::vector<std::uint8_t> registers;
        };
    }
}

#endif