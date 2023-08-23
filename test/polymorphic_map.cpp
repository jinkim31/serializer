#include <sync.h>
#include <cstdlib>

using namespace sync;

class Animal : public Serializable
{
public:
    Animal()
    {
        mName = "unnamed";
    }
    virtual void listSync() override
    {
        sync(&mName, "name");
    }
    virtual void print() =0;
protected:
    std::string mName;
};

class Dog : public Animal
{
public:
    Dog()
    {
        mDogFeature = std::rand();
        mName = "Dog"+std::to_string(mDogFeature);
    }

    virtual void listSync() override
    {
        Animal::listSync();
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
    Cat()
    {
        mCatFeature = std::rand();
        mName = "Cat"+std::to_string(mCatFeature);
    }

    virtual void listSync() override
    {
        Animal::listSync();
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
    std::srand(0);

    SerializablePolymorphicMap<int, Animal> map;
    map.setFactory(factory);
    map.get().insert({0, {std::make_shared<Dog>(), "Dog"}});
    map.get().insert({1, {std::make_shared<Cat>(), "Cat"}});
    for(const auto& animal : map.get())
    {
        std::cout<<"id "<<animal.first<<": ";
        animal.second.ptr->print();
    }

    nlohmann::json save;
    map.save(save);
    std::cout<<std::setw(4)<<save<<std::endl;

    SerializablePolymorphicMap<int, Animal> mapLoad;
    mapLoad.setFactory(factory);
    mapLoad.load(save);
    for(const auto& animal : mapLoad.get())
    {
        std::cout<<"id "<<animal.first<<": ";
        animal.second.ptr->print();
    }
    return 0;
}