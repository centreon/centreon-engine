#ifndef CCE_CONFIGURATION_DOWNTIME_HH
#  define CCE_CONFIGURATION_DOWNTIME_HH

#  include <list>
#  include <string>
#  include "com/centreon/engine/objects/timeperiod.hh"
#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/engine/configuration/object.hh"
#  include "com/centreon/shared_ptr.hh"

CCE_BEGIN()

namespace                configuration {
  class                  downtime : public object {
  public:
    enum                 type_id {
      service = 1,
      host = 2
    };

    // As a differing downtime is always a new downtime,
    // the key of a downtime is itself.
    typedef downtime key_type;

                         downtime(type_id type);
                         downtime(downtime const& right);
                         ~downtime() throw ();
    downtime&            operator=(downtime const& right);
    bool                 operator==(downtime const& right) const throw ();
    bool                 operator!=(downtime const& right) const throw ();
    bool                 operator<(downtime const& right) const throw ();
    key_type const&      key() const throw ();
    bool                 parse(char const* key, char const* value);

    std::string const&   author() const throw ();
    std::string const&   comment_data() const throw ();
    unsigned long        downtime_id() const throw ();
    type_id              downtime_type() const throw ();
    unsigned long        duration() const throw ();
    time_t               end_time() const throw ();
    time_t               entry_time() const throw ();
    bool                 fixed() const throw ();
    std::string const&   host_name() const throw ();
    std::string const&   service_description() const throw ();
    time_t               start_time() const throw ();
    unsigned long        triggered_by() const throw ();
    unsigned long        recurring_interval() const throw();
    ::timeperiod*        recurring_period() const throw();
    std::string const&   recurring_period_name() const throw();
    bool                 resolve_recurring_period();
    void                 check_validity() const;
    void                 merge(object const& obj);

  private:
    struct               setters {
      char const*        name;
      bool               (*func)(downtime&, char const*);
    };

    bool                 _set_author(std::string const& value);
    bool                 _set_comment_data(std::string const& value);
    bool                 _set_downtime_id(unsigned long value);
    bool                 _set_duration(unsigned long value);
    bool                 _set_end_time(time_t value);
    bool                 _set_entry_time(time_t value);
    bool                 _set_fixed(bool value);
    bool                 _set_host_name(std::string const& value);
    bool                 _set_service_description(std::string const& value);
    bool                 _set_start_time(time_t value);
    bool                 _set_triggered_by(unsigned long value);
    bool                 _set_recurring_interval(unsigned long value);
    bool                 _set_recurring_period(::timeperiod* value);
    bool                 _set_recurring_period_name(std::string const& value);

    std::string          _author;
    std::string          _comment_data;
    unsigned long        _downtime_id;
    type_id              _downtime_type;
    unsigned long        _duration;
    time_t               _end_time;
    time_t               _entry_time;
    bool                 _fixed;
    std::string          _host_name;
    std::string          _service_description;
    static setters const _setters[];
    time_t               _start_time;
    unsigned long        _triggered_by;
    unsigned long        _recurring_interval;
    ::timeperiod*        _recurring_period;
    std::string          _recurring_period_name;
  };

  /*typedef shared_ptr<downtime> downtime_ptr;
  typedef std::list<downtime_ptr> list_downtime;*/
}

CCE_END()

#endif // !CCE_CONFIGURATION_DOWNTIME_HH
