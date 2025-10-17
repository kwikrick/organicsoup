

# New rules idea 1:

Atoms still have color and state.

But there are two types of rules. 

## Pairwise bonding rules:

For any pair of atoms, a bonding strenght an distance are given. 

If no rule is given for a pair, there is no bond. If there was a bond, it will be broken. 
A streng of 0 also means no bond.

So existance of a bond, it distance and strength are given by a function that is determined 
for any pair of neighbouring atoms.

bond_rule(a.color, a.state, b.color, b.state) -> strength, distance. 

Strengh could also be fixed, just omit the rule to have no bond. A fixed strength
might be more suitable for a simpler and more stable simulation. 
Variabe distance is essential, because it allows to create movement, i.e. muscles.

A discrete distance: (near, far, ouf of range), might also work fine. Zero and large values are not desirable for 
stability of the physics emulation. 

Improvement: use variables X and Y and constrains on states to formulate more compactly:

a0 a0 -> strong near
a0 X0 -> strong, far
a0 X(T>=1) -> weak, outrange
X(T) Y(S=T) -> strong, near

Problem: a pair may match more than one rule. Use first or last? Error or warning?

## State rules:

State of an atom is determined by it's own color and the color of neighbouring atoms. 
Note: not the state of it's neighbours. This is important, because the function is 
stable. Rules can be evaluated in any order. 

state_rule(a.color, [list of neighbour colors]) -> a.state

if no state rule is given for an atom color a a list of neighbour colors, state is zero?
Or state simply remains unchnanged?

What if the list does not exactly match neighbours? E.g. atom has 3 neighbours, 
but ony one rule with 2 neighbours? 
(Althoug there will probably be a moment when it has two neighbours before it has 3, but not certainly)

In any case, order of list should not matter. But colors could ocur multiple times. 

Idea: list is interpretet as a multi-set. 

Idea 2: not a list but a constraint on number of each color, like:

(red == 0 & blue == 1 & green <= 3) || yellow == blue) -> red

Problem: may match more than one rule. Use first or last? Error or warning?

## Problems

Something that is not naturally expessed with this is limiting the number of bonds. 

```  
Bonding rules:
a0 a0 near

State rules:
a (a==2) a1 
```

First an a0a0 pair will be formed. 
But the state rule will result in the bond being broken (or not even established) as soon as a a third a appraches the pair. 

Maybe the current bonding rules should only create bonds, but do break. For that we need anaother rule

```
a1 a1 break
```

Alternative solution: 
 - add a number of bonds to the bonding rule? Doesn't make much sense, because they're pairs.
 - add a rule for number of bonds? a0 -> 3*a ; a1 -> 2*a + 1*b ?




# New rule idea 2:

Keep basic rules, but add charge + or -.

Charge: based on atom type?

First: simply have a range for charge. Linear or quadratic descreasing to 0 at max distance.  

Later: infinite range (allowing influence of large charged masses at distance)

More ideas: 
 - also depending on state. But will need to add ad rules, because infinitly many states
 - incorporate in the bonding rule. 

a0+b0 -> a1(+1)a2(-1)

or 

a.charge = function(state)

a: s>3?1:-1
b: s%2==0

# new rule idea 3:

specify angles for triples:

b1-a0-b1: 30 degrees
c0-a0-b2: 60 degrees
etc


An angle can also be expressed as a distance, i.e it's like a bond itself.  















