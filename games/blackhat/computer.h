#pragma once
#include <cstdint>
#include <array>

#if 0
/*
 * .CODE-----------.STK-.MEM-.IN--.
 * |00 const ddd   |ddd |ddd |ddd |
 * |01 dup         |ddd |ddd |ddd |
 * |02 drop        |ddd |ddd |... |
 * |03 add         |ddd |ddd |... |
 * |04 sub         |ddd |ddd |... |
 * |05 jlt   ddd   |ddd |ddd |... |
 * |06 jeq   ddd   |... |ddd |... |
 * |07 jgt   ddd   |... |ddd |... |
 * |08 jle   ddd   |... |ddd |... |
 * |09 jge   ddd   |... |ddd |... | 
 * |10 jne   ddd   |... |ddd |... |
 * |11 jmp   ddd   |... |ddd |OUT-|
 * |12 in    ddd   |... |ddd |ddd |
 * |13 out   ddd   |... |ddd |ddd |
 * |14 load  ddd   |... |ddd |... |
 * |15 store ddd   |... |ddd |... |
 * |16 nop         |... |ddd |... |
 * |17 and         |... |ddd |... |
 * |18 or          |... |ddd |... |
 * |19 not         |... |ddd |... |
 * |20 call        |... |ddd |... |
 * '---------------'----'----'----'
 *  [Run]  [sTep]  [Stop]  [rEset]
 *
 * Instructions:
 *     NOP
 *     CONST   DUP     DROP
 *     ADD     SUB     AND     OR      NOT     XOR
 *     CALL    RET
 *     LOAD    STORE   IN      OUT
 *     JMP     JLT     JLE     JEQ     JNE     JGE     JGT
 */
#endif

struct machine_t {
    
    static const size_t _LINES = 21;
    static const size_t _WIDTH = 11;
    static const size_t _STACK = 21;
    static const size_t _MEMORY = 21;
    static const size_t _IN = 10;
    static const size_t _OUT = 10;

    typedef std::array<uint8_t, _WIDTH> line_t;
    typedef std::array<line_t, _LINES> code_t;
    typedef std::array<uint8_t, _STACK> stack_t;
    typedef std::array<uint8_t, _MEMORY> memory_t;
    typedef std::array<uint8_t, _IN> in_t;
    typedef std::array<uint8_t, _OUT> out_t;

    uint8_t pc_;
    code_t code_;
    stack_t stack_;
    memory_t memory_;
    in_t in_;
    out_t out_;
};
