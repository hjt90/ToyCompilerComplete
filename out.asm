.data
T16:
.word 4
.text
lui $sp,0x1002
j main
demo:
sw $ra 4($sp)
lw $s7 8($sp)
addi $s6 $zero 2
add $s7 $s7 $s6
addi $s5 $zero 2
mul $s7 $s7 $s5
sw $s7 12($sp)
add $v0 $zero $s7
lw $ra 4($sp)
jr $ra
main:
addi $s7 $zero 3
addi $s6 $zero 0
mul $s6 $s6 4
sw $s7 T16($s6)
addi $s5 $zero 0
mul $s5 $s5 4
lw $s4 T16($s5)
addi $s3 $zero 1
add $s2 $s4 $s3
addi $s1 $zero 1
mul $s1 $s1 4
sw $s2 T16($s1)
addi $s2 $zero 1
mul $s2 $s2 4
lw $s0 T16($s2)
add $s2 $s4 $s0
sw $s0 8($sp)
addi $s0 $zero 2
mul $s0 $s0 4
sw $s2 T16($s0)
addi $s2 $zero 2
mul $s2 $s2 4
lw $s0 T16($s2)
sw $s0 20($sp)
sw $sp 12($sp)
addi $sp $sp 12
jal demo
lw $sp 0($sp)
sw $s4 20($sp)
lw $s4 8($sp)
sw $s4 24($sp)
sw $v0 28($sp)
sw $sp 12($sp)
addi $sp $sp 12
jal program
lw $sp 0($sp)
addi $s4 $zero 3
mul $s4 $s4 4
sw $v0 T16($s4)
addi $s0 $zero 3
mul $s0 $s0 4
lw $s0 T16($s0)
sw $s0 12($sp)
add $v0 $zero $s0
j end
program:
sw $ra 4($sp)
addi $s7 $zero 0
lw $s6 12($sp)
lw $s5 16($sp)
add $s4 $s6 $s5
sw $s7 20($sp)
lw $s3 8($sp)
bgt $s3 $s4 Label2
Label1:
J Label3
Label2:
lw $s7 12($sp)
lw $s6 16($sp)
mul $s7 $s7 $s6
addi $s6 $zero 1
add $s7 $s7 $s6
lw $s5 8($sp)
add $s5 $s5 $s7
sw $s5 24($sp)
J Label4
Label3:
lw $s7 8($sp)
sw $s7 24($sp)
Label4:
lw $s7 20($sp)
addi $s6 $zero 100
ble $s7 $s6 Label6
Label5:
J Label7
Label6:
lw $s7 24($sp)
addi $s6 $zero 2
mul $s7 $s7 $s6
sw $s7 20($sp)
sw $s7 24($sp)
J Label4
Label7:
lw $v0 20($sp)
lw $ra 4($sp)
jr $ra
end:
