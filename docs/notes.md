> [!IMPORTANT] Implementation
> Looking at parsing each object as either a number, string, or other object.
> I need to change/pick a model for handling object ownership, either some sort of global linked list where each new parsed object is set as the next pointer from the current node. Or maybe make a document object that points to the head/root node of the JSON and then each node can point to the next one.
> I need to figure out the nesting, beyond just each object's value can be another object with a value. And I need to figure out the array thing.

> [!QUESTION] Questions
> How am I going to handle multiple key value pairs in a single object?
> > Maybe with another object field subelement or something?
> > object is a linked list of subelements (each with a name and value and pointer to the next one) and a pointer to the next actual element?
