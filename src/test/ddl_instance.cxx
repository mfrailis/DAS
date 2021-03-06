
#include "ddl_instance.hxx"
#include "ddl_instance-odb.hxx"

const std::vector<lazy_shared_ptr<TestLogImage> >&
TestLog::images() const
{
  if (images_.size() > 0 and not (&(images_[0]))->loaded())
    {
      for (std::vector<lazy_shared_ptr<TestLogImage> >::const_iterator i (images_.begin());
	   i != images_.end(); ++i)
	{
	  i->load();
	}
    }
  return images_;
}


std::vector<lazy_shared_ptr<TestLogImage> >&
TestLog::images()
{
  if (images_.size() > 0 and not (&(images_[0]))->loaded())
    {
      for (std::vector<lazy_shared_ptr<TestLogImage> >::iterator i (images_.begin());
	   i != images_.end(); ++i)
	{
	  i->load();
	}
    }
  return images_;
}
