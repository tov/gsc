#pragma once

#include "../../Session.h"

#include <Wt/Http/Request.h>
#include <Wt/Http/Response.h>
#include <Wt/WResource.h>

namespace api {

class Endpoint : public Wt::WResource
{
public:
    explicit Endpoint(Wt::Dbo::SqlConnectionPool& pool);

    void handleRequest(Wt::Http::Request const&,
                       Wt::Http::Response&) override;

private:
    Wt::Dbo::SqlConnectionPool& pool_;
};

} // end namespace api
