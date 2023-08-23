#include <sync.h>

using namespace sync;

class Animal : public Serializable
{
public:
    Animal()
    {
        mName = "unnamed";
    }
    virtual void print() =0;
    void setName(const std::string& name)
    {
        mName = name;
    }
protected:
    std::string mName;
};

class Dog : public Animal
{
public:
    virtual void listSync() override
    {
        sync(&mName, "name");
        sync(&mDogFeature, "dog_feature");
    }

    virtual void print() override
    {
        std::cout<<mName<<"(Dog)"<<std::endl;
    }
private:
    int mDogFeature;
};


class Cat : public Animal
{
public:
    virtual void listSync() override
    {
        sync(&mName, "name");
        sync(&mCatFeature, "cat_feature");
    }

    virtual void print() override
    {
        std::cout<<mName<<"(Cat)"<<std::endl;
    }
private:
    int mCatFeature;
};

std::shared_ptr<Animal> factory(const std::string& typeName)
{
    if(typeName == "Dog")
        return std::make_shared<Dog>();
    if(typeName == "Cat")
        return std::make_shared<Cat>();
    return nullptr;
}

int main()
{
    SerializablePolymorphicVector<Animal> vec(factory);
    vec.get().push_back({std::make_shared<Dog>(), "Dog"});
    vec.get().push_back({std::make_shared<Cat>(), "Cat"});
    for(const auto& animal : vec.get())
    {
        animal.ptr->print();
    }

    json save;
    vec.save(save);
    std::cout<<save<<std::endl;

    SerializablePolymorphicVector<Animal> vecLoad(factory);
    vecLoad.load(save);
    for(const auto& animal : vecLoad.get())
    {
        animal.ptr->print();
    }
    return 0;
}